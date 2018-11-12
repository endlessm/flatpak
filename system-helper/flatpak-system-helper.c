/*
 * Copyright © 2014 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include <polkit/polkit.h>
#include <sys/mount.h>
#include <gio/gunixmounts.h>

#include "flatpak-dbus-generated.h"
#include "flatpak-dir-private.h"
#include "flatpak-oci-registry-private.h"
#include "flatpak-error.h"
#include "flatpak-utils-private.h"

static PolkitAuthority *authority = NULL;
static FlatpakSystemHelper *helper = NULL;
static GMainLoop *main_loop = NULL;
static guint name_owner_id = 0;

G_LOCK_DEFINE (ongoing_pulls);
static GPtrArray *ongoing_pulls = NULL;

static gboolean on_session_bus = FALSE;
static gboolean no_idle_exit = FALSE;

#define IDLE_TIMEOUT_SECS 10 * 60
#define MAX_PULLS_PER_USER 16
#define MOUNT_TIMEOUT 15

/* This uses a weird Auto prefix to avoid conflicts with later added polkit types.
 */
typedef PolkitAuthorizationResult AutoPolkitAuthorizationResult;
typedef PolkitDetails             AutoPolkitDetails;
typedef PolkitSubject             AutoPolkitSubject;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AutoPolkitAuthorizationResult, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AutoPolkitDetails, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AutoPolkitSubject, g_object_unref)

typedef struct
{
  FlatpakSystemHelper *object;
  GDBusMethodInvocation *invocation;
  GCancellable *cancellable;

  guint watch_id;
  uid_t uid;

  gint ref_count;

  const gchar *ref;
  const gchar *commit;
  gchar *root_dir_name;
  gchar *user_dir_name;
  gchar *child_repo_path;

  GUnixMountMonitor *mount_monitor;
  gulong mount_monitor_id;
  guint unsuccessful_mount_timeout_id;
  GSubprocess *bindfs;
} FlatpakSystemHelperOngoingPull;

static void
system_helper_ongoing_pull_free (FlatpakSystemHelperOngoingPull *pull)
{
  g_autoptr(GFile) root_dir = g_file_new_for_path (pull->root_dir_name);
  g_autoptr(GFile) user_dir = g_file_new_for_path (pull->user_dir_name);

  g_cancellable_cancel (pull->cancellable);

  g_clear_object (&pull->bindfs);

  flatpak_rm_rf (root_dir, NULL, NULL);
  flatpak_rm_rf (user_dir, NULL, NULL);

  g_clear_object (&pull->mount_monitor);

  g_clear_pointer (&pull->root_dir_name, g_free);
  g_clear_pointer (&pull->user_dir_name, g_free);
  g_clear_pointer (&pull->child_repo_path, g_free);

  G_LOCK (ongoing_pulls);
  g_ptr_array_remove (ongoing_pulls, pull);
  G_UNLOCK (ongoing_pulls);

  g_slice_free (FlatpakSystemHelperOngoingPull, pull);
}

static void
system_helper_ongoing_pull_teardown (FlatpakSystemHelperOngoingPull *pull)
{
  if (pull->bindfs != NULL)
    {
      /* We are probably here because graceful unmounting did not happen during handle_deploy.
         Hence, we try force unmount our bindfs directory, to make sure no open FDs can write
         to it anymore. If this fails, we kill the subprocess and try to cleanup /etc/mtab
         mount entry. */
      if (umount2 (pull->user_dir_name, MNT_FORCE) == -1)
        {
          g_autoptr(GError) local_error = NULL;
          glnx_throw_errno_prefix (&local_error, "Failed to unmount bindfs dir %s", pull->user_dir_name);
          g_warning ("%s; killing subprocess", local_error->message);
          g_clear_error (&local_error);

          g_subprocess_force_exit (pull->bindfs);
          /* Perform a lazy unmount operation to cleanup the dangling mount entry in /etc/mtab due to SIGKILL. */
          if (umount2 (pull->user_dir_name, MNT_DETACH) == -1)
            {
              glnx_throw_errno_prefix (&local_error, "Failed to umount2 bindfs dir %s", pull->user_dir_name);
              g_warning ("%s; mount maybe dangling in /etc/mtab", local_error->message);
            }
        }
    }

  if (pull->watch_id)
    g_clear_handle_id (&pull->watch_id, g_source_remove);

  if (pull->unsuccessful_mount_timeout_id)
    g_clear_handle_id (&pull->unsuccessful_mount_timeout_id, g_source_remove);

  if (pull->mount_monitor_id)
    {
      g_signal_handler_disconnect (pull->mount_monitor, pull->mount_monitor_id);
      pull->mount_monitor_id = 0;
    }
}

static void
system_helper_ongoing_pull_unref (FlatpakSystemHelperOngoingPull *pull)
{
  g_return_if_fail (pull != NULL);
  g_return_if_fail (pull->ref_count > 0);

  if (g_atomic_int_dec_and_test (&pull->ref_count))
    {
      system_helper_ongoing_pull_teardown (pull);
      system_helper_ongoing_pull_free (pull);
    }
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (FlatpakSystemHelperOngoingPull, system_helper_ongoing_pull_unref);

static FlatpakSystemHelperOngoingPull *
system_helper_ongoing_pull_ref (FlatpakSystemHelperOngoingPull *pull)
{
  g_return_val_if_fail (pull != NULL, NULL);
  g_return_val_if_fail (pull->ref_count > 0, NULL);

  g_atomic_int_inc (&pull->ref_count);
  return pull;
}

static void
name_vanished_callback (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  g_autoptr(FlatpakSystemHelperOngoingPull) pull = user_data;
}

static void
mounts_changed (GUnixMountMonitor *monitor, gpointer user_data)
{
  g_autoptr(FlatpakSystemHelperOngoingPull) pull = user_data;
  g_autoptr(GList) mounts = NULL;
  GUnixMountEntry *mount_entry = NULL;
  GList *l = NULL;
  gboolean mount_successful = FALSE;

  mounts = g_unix_mounts_get (NULL);
  for (l = mounts; l != NULL; l = l->next)
    {
      mount_entry = (GUnixMountEntry *) l->data;
      if (g_strcmp0 (pull->user_dir_name, g_unix_mount_get_mount_path (mount_entry)) == 0)
        mount_successful = TRUE;
      g_unix_mount_free (mount_entry);
    }

  if (mount_successful)
    {
      g_autofree gchar *basename = NULL;
      g_autofree gchar *child_repo_path = NULL;

      g_signal_handler_disconnect (monitor, pull->mount_monitor_id);
      pull->mount_monitor_id = 0;

      basename = g_path_get_basename (pull->child_repo_path);
      child_repo_path = g_build_filename (pull->user_dir_name, basename, NULL);

      flatpak_system_helper_complete_get_child_repo_for_pull (pull->object,
                                                              pull->invocation,
                                                              child_repo_path);

      g_source_remove (pull->unsuccessful_mount_timeout_id);
      pull->unsuccessful_mount_timeout_id = 0;
    }
}

static gboolean
unsuccessful_mount_timeout (gpointer user_data)
{
  FlatpakSystemHelperOngoingPull *pull = user_data;
  g_autoptr(GError) error = NULL;

  system_helper_ongoing_pull_teardown (pull);
  g_set_error (&error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
               "Failed to mount %s on %s with bindfs", pull->user_dir_name, pull->root_dir_name);
  g_dbus_method_invocation_return_gerror (pull->invocation, error);

  return FALSE;
}

static FlatpakSystemHelperOngoingPull *
system_helper_ongoing_pull_new (FlatpakSystemHelper   *object,
                                GDBusMethodInvocation *invocation,
                                const gchar           *ref,
                                const gchar           *commit,
                                uid_t                  uid,
                                gchar                 *user_dir_name,
                                gchar                 *root_dir_name,
                                gchar                 *child_repo_path)
{
  GDBusConnection *connection = g_dbus_method_invocation_get_connection (invocation);
  FlatpakSystemHelperOngoingPull *pull;

  pull = g_slice_new0 (FlatpakSystemHelperOngoingPull);
  pull->object = object;
  pull->invocation = invocation;
  pull->ref = ref;
  pull->commit = commit;
  pull->cancellable = g_cancellable_new ();
  pull->uid = uid;
  pull->ref_count = 1;

  pull->watch_id = g_bus_watch_name_on_connection (connection,
                                                   g_dbus_connection_get_unique_name (connection),
                                                   G_BUS_NAME_WATCHER_FLAGS_NONE, NULL,
                                                   name_vanished_callback,
                                                   system_helper_ongoing_pull_ref (pull), NULL);

  pull->mount_monitor = g_unix_mount_monitor_get ();
  pull->mount_monitor_id = g_signal_connect (pull->mount_monitor,
                                             "mounts-changed",
                                             G_CALLBACK (mounts_changed),
                                             system_helper_ongoing_pull_ref (pull));

  pull->unsuccessful_mount_timeout_id = g_timeout_add_seconds (MOUNT_TIMEOUT,
                                                               unsuccessful_mount_timeout,
                                                               pull);

  pull->user_dir_name = g_strdup (user_dir_name);
  pull->root_dir_name = g_strdup (root_dir_name);
  pull->child_repo_path = g_strdup (child_repo_path);

  return pull;
}

static void
skeleton_died_cb (gpointer data)
{
  g_debug ("skeleton finalized, exiting");
  g_main_loop_quit (main_loop);
}

static gboolean
unref_skeleton_in_timeout_cb (gpointer user_data)
{
  static gboolean unreffed = FALSE;

  g_debug ("unreffing helper main ref");
  if (!unreffed)
    {
      g_object_unref (helper);
      unreffed = TRUE;
    }

  return G_SOURCE_REMOVE;
}

static void
unref_skeleton_in_timeout (void)
{
  if (name_owner_id)
    g_bus_unown_name (name_owner_id);
  name_owner_id = 0;

  /* After we've lost the name or idled we drop the main ref on the helper
     so that we'll exit when it drops to zero. However, if there are
     outstanding calls these will keep the refcount up during the
     execution of them. We do the unref on a timeout to make sure
     we're completely draining the queue of (stale) requests. */
  g_timeout_add (500, unref_skeleton_in_timeout_cb, NULL);
}

static gboolean
idle_timeout_cb (gpointer user_data)
{
  G_LOCK (ongoing_pulls);
  guint ongoing_pulls_len = ongoing_pulls->len;
  G_UNLOCK (ongoing_pulls);
  if (ongoing_pulls_len != 0)
    return G_SOURCE_CONTINUE;

  if (name_owner_id)
    {
      g_debug ("Idle - unowning name");
      unref_skeleton_in_timeout ();
    }
  return G_SOURCE_REMOVE;
}

G_LOCK_DEFINE_STATIC (idle);
static void
schedule_idle_callback (void)
{
  static guint idle_timeout_id = 0;

  G_LOCK (idle);

  if (!no_idle_exit)
    {
      if (idle_timeout_id != 0)
        g_source_remove (idle_timeout_id);

      idle_timeout_id = g_timeout_add_seconds (IDLE_TIMEOUT_SECS, idle_timeout_cb, NULL);
    }

  G_UNLOCK (idle);
}

static FlatpakDir *
dir_get_system (const char *installation, GError **error)
{
  FlatpakDir *system = NULL;

  if (installation != NULL && *installation != '\0')
    system = flatpak_dir_get_system_by_id (installation, NULL, error);
  else
    system = flatpak_dir_get_system_default ();

  /* This can happen in case of error with flatpak_dir_get_system_by_id(). */
  if (system == NULL)
    return NULL;

  flatpak_dir_set_no_system_helper (system, TRUE);
  return system;
}

static void
no_progress_cb (OstreeAsyncProgress *progress, gpointer user_data)
{
}

static gboolean
get_connection_uid (GDBusMethodInvocation *invocation, uid_t *uid_out, GError **error)
{
  GDBusConnection *connection = g_dbus_method_invocation_get_connection (invocation);
  const gchar *sender = g_dbus_method_invocation_get_sender (invocation);
  g_autoptr(GVariant) credentials = NULL;
  uid_t uid;

  credentials = g_dbus_connection_call_sync (connection,
                                             "org.freedesktop.DBus",
                                             "/org/freedesktop/DBus",
                                             "org.freedesktop.DBus",
                                             "GetConnectionCredentials",
                                             g_variant_new ("(s)", sender),
                                             G_VARIANT_TYPE ("(a{sv})"), G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, error);
  if (credentials == NULL)
    return FALSE;

  g_autoptr(GVariant) dict = g_variant_get_child_value (credentials, 0);
  g_autoptr(GVariant) unixUserId = g_variant_lookup_value (dict, "UnixUserID", G_VARIANT_TYPE_UINT32);

  if (unixUserId == NULL)
    {
      g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                   "Failed to query UnixUserId for the bus name: %s", sender);
      return FALSE;
    }

  *uid_out = g_variant_get_uint32 (unixUserId);
  return TRUE;
}

static guint
get_uid_pull_count (uid_t uid)
{
  FlatpakSystemHelperOngoingPull *pull;
  guint uid_pull_count = 0;

  G_LOCK (ongoing_pulls);
  for (guint i = 0; i < ongoing_pulls->len; i++)
    {
      pull = g_ptr_array_index (ongoing_pulls, i);
      if (pull->uid == uid)
        uid_pull_count++;
    }
  G_UNLOCK (ongoing_pulls);

  return uid_pull_count;
}

static void
subprocess_wait_check_cb (GObject      *source_object,
                          GAsyncResult *result,
                          gpointer      user_data)
{
  GSubprocess *subprocess = (GSubprocess *) source_object;
  g_autoptr(FlatpakSystemHelperOngoingPull) pull = user_data;
  g_autoptr(GError) error = NULL;

  if (!g_subprocess_wait_check_finish (subprocess, result, &error))
    {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
          g_dbus_method_invocation_return_gerror (pull->invocation, error);
          system_helper_ongoing_pull_teardown (pull);
        }
    }
}

static gboolean
check_for_repo_containing_partial_pull (GFile *cache_dir, const gchar *commit, gchar **out_repo_location)
{
  g_autoptr(GFileEnumerator) enumerator = NULL;
  GFileInfo *file_info = NULL;
  g_autoptr(GError) error = NULL;
  const gchar *name;

  enumerator = g_file_enumerate_children (cache_dir,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                          G_FILE_QUERY_INFO_NONE, NULL, &error);
  if (enumerator == NULL)
    {
      g_warning ("Failed to get file enumerator: %s", error->message);
      return FALSE;
    }

  while (TRUE)
    {
      if (!g_file_enumerator_iterate (enumerator, &file_info, NULL, NULL, &error))
        {
          g_warning ("Error while file enumerator iterating: %s", error->message);
          break;
        }

      if (file_info == NULL)
        break;

      name = g_file_info_get_name (file_info);
      if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY &&
          g_str_has_prefix (name, "repo-"))
        {
          g_autoptr(GFile) repo_file = g_file_get_child (cache_dir, name);
          g_autoptr(GFile) state = g_file_get_child (repo_file, "state");
          g_autofree gchar *commit_partial_basename = g_strdup_printf ("%s.commitpartial", commit);
          g_autoptr(GFile) commit_partial = g_file_get_child (state, commit_partial_basename);

          if (g_file_query_exists (commit_partial, NULL))
            {
              g_autoptr(OstreeRepo) repo = ostree_repo_new (repo_file);
              g_autofree gchar *repo_path = g_file_get_path (repo_file);
              /* Validate repo before returning */
              if (!ostree_repo_open (repo, NULL, &error))
                {
                  g_warning ("Partial pull available at %s, but invalid repo: %s", repo_path, error->message);
                  return FALSE;
                }
              else
                {
                  *out_repo_location = g_strdup (repo_path);
                  return TRUE;
                }
            }
        }
    }

  return FALSE;
}

static gboolean
check_partial_pull_availability (const gchar *repo_tmp,
                                 const gchar *ref_basename,
                                 const gchar *commit,
                                 gchar      **out_location)
{
  g_autoptr(GFileEnumerator) enumerator = NULL;
  g_autoptr(GFile) repo_tmpfile = NULL;
  GFileInfo *file_info = NULL;
  g_autofree gchar *prefix = NULL;
  g_autoptr(GError) error = NULL;
  const gchar *name;

  g_debug ("Checking partial pull availability for ref %s (commit: %s)", ref_basename, commit);

  repo_tmpfile = g_file_new_for_path (repo_tmp);
  enumerator = g_file_enumerate_children (repo_tmpfile,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                          G_FILE_QUERY_INFO_NONE, NULL, NULL);
  if (enumerator == NULL)
    {
      g_warning ("Failed to get file enumerator: %s", error->message);
      return FALSE;
    }

  prefix = g_strdup_printf ("flatpak-cache-%s", ref_basename);

  /* We are looking for flatpak-cache-$ref-XXXXXX directory in repo/tmp.
     If we find one, we try to look for an OSTree repo which possibly has
     the $commit.commitpartial file denoting the partial pull for that $commit. */
  while (TRUE)
    {
      if (!g_file_enumerator_iterate (enumerator, &file_info, NULL, NULL, &error))
        {
          g_warning ("Error while file enumerator iterating: %s", error->message);
          break;
        }

      if (file_info == NULL)
        break;

      name = g_file_info_get_name (file_info);
      if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY &&
          g_str_has_prefix (name, prefix))
        {
          g_autoptr(GFile) cache_dir = g_file_get_child (repo_tmpfile, name);

          if (check_for_repo_containing_partial_pull (cache_dir, commit, out_location))
            return TRUE;
        }
    }

  return FALSE;
}

static gboolean
handle_get_child_repo_for_pull (FlatpakSystemHelper   *object,
                                GDBusMethodInvocation *invocation,
                                const gchar           *ref,
                                const gchar           *commit,
                                const gchar           *arg_installation)
{
  GDBusConnection *connection = g_dbus_method_invocation_get_connection (invocation);
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree gchar *bindfs_path = NULL;


  g_debug ("GetChildRepoForPull %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  bindfs_path = g_find_program_in_path ("bindfs");
  if (bindfs_path == NULL)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
                                             "Can't find bindfs in the system.");
      return TRUE;
    }
  else
    {
      g_autoptr(FlatpakSystemHelperOngoingPull) new_pull = NULL;
      g_autofree gchar *child_repo_path = NULL;
      g_autofree gchar *user_dir = NULL;
      g_autofree gchar *root_dir = NULL;
      g_autofree gchar *flatpak_dir = NULL;
      g_autofree gchar *arg_user = NULL;
      g_autofree gchar *out_location = NULL;
      guint uid_pull_count = 0;
      uid_t uid;

      if (!get_connection_uid (invocation, &uid, &error))
        {
          g_dbus_method_invocation_return_gerror (invocation, error);
          return TRUE;
        }

      uid_pull_count = get_uid_pull_count (uid);
      if (uid_pull_count > MAX_PULLS_PER_USER)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_LIMITS_EXCEEDED,
                                                 "Maximum number of pulls per user exceeded for UID: %u", uid);
          return TRUE;
        }

      flatpak_dir = g_file_get_path (flatpak_dir_get_path (system));
      g_autofree gchar *repo_tmp = g_build_filename (flatpak_dir, "repo", "tmp", NULL);

      g_auto(GStrv) parts = flatpak_decompose_ref (ref, NULL);
      if (check_partial_pull_availability (repo_tmp, parts[1], commit, &out_location))
        {
          g_autoptr(GFile) user_dir_file = NULL;

          g_debug ("Found a partial pull for ref %s at: %s", parts[1], out_location);
          root_dir = g_path_get_dirname (out_location);
          user_dir = g_strdup_printf ("%s-bindfs", root_dir);
          user_dir_file = g_file_new_for_path (user_dir);
          if (!g_file_query_exists (user_dir_file, NULL))
            g_mkdir_with_parents (user_dir, 0755);
          child_repo_path = g_steal_pointer (&out_location);
        }
      else
        {
          g_autoptr(OstreeRepo) child_repo = NULL;
          g_auto(GLnxLockFile) child_repo_lock = { 0, };

          g_autofree gchar *cache_dir_basename = g_strdup_printf ("flatpak-cache-%s-XXXXXX", parts[1]);
          root_dir = g_mkdtemp_full (g_build_filename (repo_tmp, cache_dir_basename, NULL), 0755);
          user_dir = g_strdup_printf ("%s-bindfs", root_dir);
          if (g_mkdir_with_parents (user_dir, 0755) == -1)
            {
              glnx_throw_errno_prefix (&error, "Can't create directory for bindfs mount at %s", user_dir);
              g_dbus_method_invocation_return_gerror (invocation, error);
              return TRUE;
            }

          g_autoptr(GFile) root_dir_file = g_file_new_for_path (root_dir);
          child_repo = flatpak_dir_create_child_repo (system, root_dir_file, &child_repo_lock, NULL, &error);
          if (child_repo == NULL)
            {
              g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                     "Failed to create OSTree child repo at %s", root_dir);
              return TRUE;
            }
          g_autoptr(GFile) repo_file = ostree_repo_get_path (child_repo);
          child_repo_path = g_file_get_path (repo_file);
        }

      g_assert (child_repo_path != NULL);

      new_pull = system_helper_ongoing_pull_new (object, invocation, ref, commit,
                                                 uid, user_dir, root_dir, child_repo_path);

      /* In bindfs, --force-user forces file-ownership to be seen in the mounted mirrored directory(i.e. user_dir),
         while --create-for-user enforces the actual owner of the file during file creation on disk.  */
      arg_user = g_strdup_printf ("--force-user=%"G_GUINT32_FORMAT, uid);
      new_pull->bindfs = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE,
                                           &error,
                                           bindfs_path,
                                           arg_user,"-f",
                                           "-o", "nonempty",
                                           "--create-for-user=root",
                                           "--create-for-group=root",
                                           root_dir, user_dir, NULL);
      if (new_pull->bindfs == NULL)
        {
          system_helper_ongoing_pull_teardown (new_pull);
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_SPAWN_FAILED,
                                                 "Error spawning bindfs for user:%"G_GUINT32_FORMAT" at: %s", uid, user_dir);
          return TRUE;
        }

      G_LOCK (ongoing_pulls);
      g_ptr_array_add (ongoing_pulls, new_pull);
      G_UNLOCK (ongoing_pulls);

      g_subprocess_wait_check_async (new_pull->bindfs,
                                     new_pull->cancellable,
                                     subprocess_wait_check_cb,
                                     system_helper_ongoing_pull_ref (new_pull));
    }

  return TRUE;
}

static FlatpakSystemHelperOngoingPull *
get_ongoing_pull_by_dir (const gchar *arg_repo_path)
{
  FlatpakSystemHelperOngoingPull *pull = NULL;
  g_autofree gchar *user_dir = g_path_get_dirname (arg_repo_path);

  G_LOCK (ongoing_pulls);
  for (guint i = 0; i < ongoing_pulls->len; i++)
    {
      pull = g_ptr_array_index (ongoing_pulls, i);
      if (g_strcmp0 (pull->user_dir_name, user_dir) == 0)
        break;
    }
  G_UNLOCK (ongoing_pulls);

  return pull;
}


static gboolean
handle_deploy (FlatpakSystemHelper   *object,
               GDBusMethodInvocation *invocation,
               const gchar           *arg_repo_path,
               guint32                arg_flags,
               const gchar           *arg_ref,
               const gchar           *arg_origin,
               const gchar *const    *arg_subpaths,
               const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GFile) path = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GFile) deploy_dir = NULL;
  g_autoptr(OstreeAsyncProgress) ostree_progress = NULL;
  gboolean is_update;
  gboolean is_oci;
  gboolean no_deploy;
  gboolean local_pull;
  gboolean reinstall;
  g_autofree char *url = NULL;
  g_autofree gchar *repo_path = NULL;
  g_autoptr(FlatpakSystemHelperOngoingPull) ongoing_pull = NULL;

  g_debug ("Deploy %s %u %s %s %s", arg_repo_path, arg_flags, arg_ref, arg_origin, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  ongoing_pull = get_ongoing_pull_by_dir (arg_repo_path);
  if (ongoing_pull != NULL)
    {
      g_autofree gchar *bindfs_repo_path = NULL;
      g_autofree gchar *real_repo_path = NULL;

      if (umount (ongoing_pull->user_dir_name) == -1)
        {
          system_helper_ongoing_pull_teardown (ongoing_pull);
          glnx_throw_errno_prefix (&error, "Unmounting %s failed", ongoing_pull->user_dir_name);
          g_dbus_method_invocation_return_gerror (invocation, error);
          return TRUE;
        }

      if (ongoing_pull->bindfs)
        {
          g_subprocess_wait (ongoing_pull->bindfs, NULL, NULL);
          g_clear_object (&ongoing_pull->bindfs);
        }

        bindfs_repo_path = g_path_get_basename (arg_repo_path);
        real_repo_path = g_build_filename (ongoing_pull->root_dir_name, bindfs_repo_path, NULL);
        path = g_file_new_for_path (real_repo_path);
        repo_path = g_steal_pointer (&real_repo_path);
    }

  if (path == NULL)
    {
      path = g_file_new_for_path (arg_repo_path);
      repo_path = g_strdup (arg_repo_path);
    }

  g_assert (repo_path != NULL);

  if ((arg_flags & ~FLATPAK_HELPER_DEPLOY_FLAGS_ALL) != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", (arg_flags & ~FLATPAK_HELPER_DEPLOY_FLAGS_ALL));
      return TRUE;
    }

  if (!g_file_query_exists (path, NULL))
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Path does not exist");
      return TRUE;
    }

  is_update = (arg_flags & FLATPAK_HELPER_DEPLOY_FLAGS_UPDATE) != 0;
  no_deploy = (arg_flags & FLATPAK_HELPER_DEPLOY_FLAGS_NO_DEPLOY) != 0;
  local_pull = (arg_flags & FLATPAK_HELPER_DEPLOY_FLAGS_LOCAL_PULL) != 0;
  reinstall = (arg_flags & FLATPAK_HELPER_DEPLOY_FLAGS_REINSTALL) != 0;

  deploy_dir = flatpak_dir_get_if_deployed (system, arg_ref, NULL, NULL);

  if (deploy_dir)
    {
      g_autofree char *real_origin = NULL;
      if (!is_update)
        {
          if (!reinstall)
            {
              /* Can't install already installed app */
              g_dbus_method_invocation_return_error (invocation, FLATPAK_ERROR, FLATPAK_ERROR_ALREADY_INSTALLED,
                                                     "%s is already installed", arg_ref);
              return TRUE;
            }
        }
      else
        {
          real_origin = flatpak_dir_get_origin (system, arg_ref, NULL, NULL);
          if (g_strcmp0 (real_origin, arg_origin) != 0)
            {
              g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                                     "Wrong origin %s for update", arg_origin);
              return TRUE;
            }
        }
    }
  else if (!deploy_dir && is_update)
    {
      /* Can't update not installed app */
      g_dbus_method_invocation_return_error (invocation, FLATPAK_ERROR, FLATPAK_ERROR_NOT_INSTALLED,
                                             "%s is not installed", arg_ref);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                             "Can't open system repo %s", error->message);
      return TRUE;
    }

  is_oci = flatpak_dir_get_remote_oci (system, arg_origin);

  if (strlen (repo_path) > 0 && is_oci)
    {
      g_autoptr(GFile) registry_file = g_file_new_for_path (repo_path);
      g_autofree char *registry_uri = g_file_get_uri (registry_file);
      g_autoptr(FlatpakOciRegistry) registry = NULL;
      g_autoptr(FlatpakOciIndex) index = NULL;
      const FlatpakOciManifestDescriptor *desc;
      g_autoptr(FlatpakOciVersioned) versioned = NULL;
      g_autoptr(FlatpakRemoteState) state = NULL;
      FlatpakCollectionRef collection_ref;
      g_autoptr(GHashTable) remote_refs = NULL;
      g_autofree char *checksum = NULL;
      const char *verified_digest;
      g_autofree char *upstream_url = NULL;

      ostree_repo_remote_get_url (flatpak_dir_get_repo (system),
                                  arg_origin,
                                  &upstream_url,
                                  NULL);

      if (upstream_url == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Remote %s is disabled", arg_origin);
          return TRUE;
        }

      registry = flatpak_oci_registry_new (registry_uri, FALSE, -1, NULL, &error);
      if (registry == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Can't open child OCI registry: %s", error->message);
          return TRUE;
        }

      index = flatpak_oci_registry_load_index (registry, NULL, &error);
      if (index == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Can't open child OCI registry index: %s", error->message);
          return TRUE;
        }

      desc = flatpak_oci_index_get_manifest (index, arg_ref);
      if (desc == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Can't find ref %s in child OCI registry index", arg_ref);
          return TRUE;
        }

      versioned = flatpak_oci_registry_load_versioned (registry, NULL, desc->parent.digest, NULL,
                                                       NULL, &error);
      if (versioned == NULL || !FLATPAK_IS_OCI_MANIFEST (versioned))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Can't open child manifest");
          return TRUE;
        }

      state = flatpak_dir_get_remote_state (system, arg_origin, NULL, &error);
      if (state == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "%s: Can't get remote state: %s", arg_origin, error->message);
          return TRUE;
        }

      /* We need to use list_all_remote_refs because we don't care about
       * enumerate vs. noenumerate.
       */
      if (!flatpak_dir_list_all_remote_refs (system, state, &remote_refs, NULL, &error))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "%s: Can't list refs: %s", arg_origin, error->message);
          return TRUE;
        }

      collection_ref.collection_id = state->collection_id;
      collection_ref.ref_name = (char *) arg_ref;

      verified_digest = g_hash_table_lookup (remote_refs, &collection_ref);
      if (!verified_digest)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "%s: ref %s not found", arg_origin, arg_ref);
          return TRUE;
        }

      if (!g_str_has_prefix (desc->parent.digest, "sha256:") ||
          strcmp (desc->parent.digest + strlen ("sha256:"), verified_digest) != 0)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "%s: manifest hash in downloaded content does not match ref %s", arg_origin, arg_ref);
          return TRUE;
        }

      checksum = flatpak_pull_from_oci (flatpak_dir_get_repo (system), registry, NULL, desc->parent.digest, FLATPAK_OCI_MANIFEST (versioned),
                                        arg_origin, arg_ref, NULL, NULL, NULL, &error);
      if (checksum == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Can't pull ref %s from child OCI registry index: %s", arg_ref, error->message);
          return TRUE;
        }
    }
  else if (strlen (repo_path) > 0)
    {
      g_autoptr(GMainContextPopDefault) main_context = NULL;

      /* Work around ostree-pull spinning the default main context for the sync calls */
      main_context = flatpak_main_context_new_default ();

      ostree_progress = ostree_async_progress_new_and_connect (no_progress_cb, NULL);

      if (!flatpak_dir_pull_untrusted_local (system, repo_path,
                                             arg_origin,
                                             arg_ref,
                                             (const char **) arg_subpaths,
                                             ostree_progress,
                                             NULL, &error))
        {
          if (error->domain == FLATPAK_ERROR)
            g_dbus_method_invocation_return_gerror (invocation, error);
          else
            g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                   "Error pulling from repo: %s", error->message);

          return TRUE;
        }

      if (ostree_progress)
        ostree_async_progress_finish (ostree_progress);
    }
  else if (local_pull)
    {
      g_autoptr(GMainContextPopDefault) main_context = NULL;

      g_autoptr(FlatpakRemoteState) state = NULL;
      if (!ostree_repo_remote_get_url (flatpak_dir_get_repo (system),
                                       arg_origin,
                                       &url,
                                       &error))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Error getting remote url: %s", error->message);
          return TRUE;
        }

      if (!g_str_has_prefix (url, "file:"))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Local pull url doesn't start with file://");
          return TRUE;
        }

      state = flatpak_dir_get_remote_state_optional (system, arg_origin, NULL, &error);
      if (state == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Error pulling from repo: %s", error->message);
          return TRUE;
        }

      /* Work around ostree-pull spinning the default main context for the sync calls */
      main_context = flatpak_main_context_new_default ();

      ostree_progress = ostree_async_progress_new_and_connect (no_progress_cb, NULL);

      if (!flatpak_dir_pull (system, state, arg_ref, NULL, NULL, (const char **) arg_subpaths, NULL,
                             FLATPAK_PULL_FLAGS_NONE, OSTREE_REPO_PULL_FLAGS_UNTRUSTED, ostree_progress,
                             NULL, &error))
        {
          if (error->domain == FLATPAK_ERROR)
            g_dbus_method_invocation_return_gerror (invocation, error);
          else
            g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                   "Error pulling from repo: %s", error->message);

          return TRUE;
        }

      if (ostree_progress)
        ostree_async_progress_finish (ostree_progress);
    }

  if (!no_deploy)
    {
      if (is_update)
        {
          if (!flatpak_dir_deploy_update (system, arg_ref,
                                          NULL, (const char **) arg_subpaths, NULL, &error))
            {
              if (g_error_matches (error, FLATPAK_ERROR, FLATPAK_ERROR_ALREADY_INSTALLED))
                g_dbus_method_invocation_return_error (invocation, FLATPAK_ERROR, FLATPAK_ERROR_ALREADY_INSTALLED,
                                                       "%s", error->message);
              else
                g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                       "Error deploying: %s", error->message);
              return TRUE;
            }
        }
      else
        {
          if (!flatpak_dir_deploy_install (system, arg_ref, arg_origin,
                                           (const char **) arg_subpaths,
                                           reinstall,
                                           NULL, &error))
            {
              g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                     "Error deploying: %s", error->message);
              return TRUE;
            }
        }
    }

  flatpak_system_helper_complete_deploy (object, invocation);

  return TRUE;
}

static gboolean
handle_deploy_appstream (FlatpakSystemHelper   *object,
                         GDBusMethodInvocation *invocation,
                         const gchar           *arg_repo_path,
                         const gchar           *arg_origin,
                         const gchar           *arg_arch,
                         const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree char *new_branch = NULL;
  g_autofree char *old_branch = NULL;
  gboolean is_oci;

  g_debug ("DeployAppstream %s %s %s %s", arg_repo_path, arg_origin, arg_arch, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (strlen (arg_repo_path) > 0)
    {
      g_autoptr(GFile) path = g_file_new_for_path (arg_repo_path);
      if (!g_file_query_exists (path, NULL))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Path does not exist");
          return TRUE;
        }
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                             "Can't open system repo %s", error->message);
      return TRUE;
    }

  is_oci = flatpak_dir_get_remote_oci (system, arg_origin);

  new_branch = g_strdup_printf ("appstream2/%s", arg_arch);
  old_branch = g_strdup_printf ("appstream/%s", arg_arch);

  if (is_oci)
    {
      /* In the OCI case, we just do the full update, including network i/o, in the
       * system helper, see comment in flatpak_dir_update_appstream()
       */
      if (!flatpak_dir_update_appstream (system,
                                         arg_origin,
                                         arg_arch,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &error))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Error updating appstream: %s", error->message);
          return TRUE;
        }

      flatpak_system_helper_complete_deploy_appstream (object, invocation);
      return TRUE;
    }
  else if (strlen (arg_repo_path) > 0)
    {
      g_autoptr(GError) first_error = NULL;
      g_autoptr(GError) second_error = NULL;
      g_autoptr(GMainContextPopDefault) main_context = NULL;

      /* Work around ostree-pull spinning the default main context for the sync calls */
      main_context = flatpak_main_context_new_default ();

      if (!flatpak_dir_pull_untrusted_local (system, arg_repo_path,
                                             arg_origin,
                                             new_branch,
                                             NULL,
                                             NULL,
                                             NULL, &first_error))
        {
          if (!flatpak_dir_pull_untrusted_local (system, arg_repo_path,
                                                 arg_origin,
                                                 old_branch,
                                                 NULL,
                                                 NULL,
                                                 NULL, &second_error))
            {
              g_prefix_error (&first_error, "Error updating appstream2: ");
              g_prefix_error (&second_error, "%s; Error updating appstream: ", first_error->message);
              g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                     "Error pulling from repo: %s", second_error->message);
              return TRUE;
            }
        }
    }
  else /* empty path == local pull */
    {
      g_autoptr(FlatpakRemoteState) state = NULL;
      g_autoptr(OstreeAsyncProgress) ostree_progress = NULL;
      g_autoptr(GError) first_error = NULL;
      g_autoptr(GError) second_error = NULL;
      g_autofree char *url = NULL;
      g_autoptr(GMainContextPopDefault) main_context = NULL;

      if (!ostree_repo_remote_get_url (flatpak_dir_get_repo (system),
                                       arg_origin,
                                       &url,
                                       &error))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Error getting remote url: %s", error->message);
          return TRUE;
        }

      if (!g_str_has_prefix (url, "file:"))
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Local pull url doesn't start with file://");
          return TRUE;
        }

      state = flatpak_dir_get_remote_state_optional (system, arg_origin, NULL, &error);
      if (state == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Error pulling from repo: %s", error->message);
          return TRUE;
        }

      /* Work around ostree-pull spinning the default main context for the sync calls */
      main_context = flatpak_main_context_new_default ();

      ostree_progress = ostree_async_progress_new_and_connect (no_progress_cb, NULL);

      if (!flatpak_dir_pull (system, state, new_branch, NULL, NULL, NULL, NULL,
                             FLATPAK_PULL_FLAGS_NONE, OSTREE_REPO_PULL_FLAGS_UNTRUSTED, ostree_progress,
                             NULL, &first_error))
        {
          if (!flatpak_dir_pull (system, state, old_branch, NULL, NULL, NULL, NULL,
                                 FLATPAK_PULL_FLAGS_NONE, OSTREE_REPO_PULL_FLAGS_UNTRUSTED, ostree_progress,
                                 NULL, &second_error))
            {
              g_prefix_error (&first_error, "Error updating appstream2: ");
              g_prefix_error (&second_error, "%s; Error updating appstream: ", first_error->message);
              g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                     "Error pulling from repo: %s", second_error->message);
              return TRUE;
            }
        }

      if (ostree_progress)
        ostree_async_progress_finish (ostree_progress);
    }

  if (!flatpak_dir_deploy_appstream (system,
                                     arg_origin,
                                     arg_arch,
                                     NULL,
                                     NULL,
                                     &error))
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                             "Error deploying appstream: %s", error->message);
      return TRUE;
    }

  flatpak_system_helper_complete_deploy_appstream (object, invocation);

  return TRUE;
}

static gboolean
handle_uninstall (FlatpakSystemHelper   *object,
                  GDBusMethodInvocation *invocation,
                  guint                  arg_flags,
                  const gchar           *arg_ref,
                  const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("Uninstall %u %s %s", arg_flags, arg_ref, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if ((arg_flags & ~FLATPAK_HELPER_UNINSTALL_FLAGS_ALL) != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", (arg_flags & ~FLATPAK_HELPER_UNINSTALL_FLAGS_ALL));
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_uninstall (system, arg_ref, arg_flags, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_uninstall (object, invocation);

  return TRUE;
}

static gboolean
handle_install_bundle (FlatpakSystemHelper   *object,
                       GDBusMethodInvocation *invocation,
                       const gchar           *arg_bundle_path,
                       guint32                arg_flags,
                       const gchar           *arg_remote,
                       const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GFile) path = g_file_new_for_path (arg_bundle_path);
  g_autoptr(GError) error = NULL;
  g_autofree char *ref = NULL;

  g_debug ("InstallBundle %s %u %s %s", arg_bundle_path, arg_flags, arg_remote, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (arg_flags != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", arg_flags);
      return TRUE;
    }

  if (!g_file_query_exists (path, NULL))
    {
      g_dbus_method_invocation_return_error (invocation, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                                             "Bundle %s does not exist", arg_bundle_path);
      return TRUE;
    }

  if (!flatpak_dir_install_bundle (system, path, arg_remote, &ref, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_install_bundle (object, invocation, ref);

  return TRUE;
}


static gboolean
handle_configure_remote (FlatpakSystemHelper   *object,
                         GDBusMethodInvocation *invocation,
                         guint                  arg_flags,
                         const gchar           *arg_remote,
                         const gchar           *arg_config,
                         GVariant              *arg_gpg_key,
                         const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GKeyFile) config = g_key_file_new ();
  g_autofree char *group = g_strdup_printf ("remote \"%s\"", arg_remote);
  g_autoptr(GBytes) gpg_data = NULL;
  gboolean force_remove;

  g_debug ("ConfigureRemote %u %s %s", arg_flags, arg_remote, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (*arg_remote == 0 || strchr (arg_remote, '/') != NULL)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Invalid remote name: %s", arg_remote);
      return TRUE;
    }

  if ((arg_flags & ~FLATPAK_HELPER_CONFIGURE_REMOTE_FLAGS_ALL) != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", (arg_flags & ~FLATPAK_HELPER_CONFIGURE_REMOTE_FLAGS_ALL));
      return TRUE;
    }

  if (!g_key_file_load_from_data (config, arg_config, strlen (arg_config),
                                  G_KEY_FILE_NONE, &error))
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Invalid config: %s\n", error->message);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (g_variant_get_size (arg_gpg_key) > 0)
    gpg_data = g_variant_get_data_as_bytes (arg_gpg_key);

  force_remove = (arg_flags & FLATPAK_HELPER_CONFIGURE_REMOTE_FLAGS_FORCE_REMOVE) != 0;

  if (g_key_file_has_group (config, group))
    {
      /* Add/Modify */
      if (!flatpak_dir_modify_remote (system, arg_remote, config,
                                      gpg_data, NULL, &error))
        {
          g_dbus_method_invocation_return_gerror (invocation, error);
          return TRUE;
        }
    }
  else
    {
      /* Remove */
      if (!flatpak_dir_remove_remote (system,
                                      force_remove,
                                      arg_remote,
                                      NULL, &error))
        {
          g_dbus_method_invocation_return_gerror (invocation, error);
          return TRUE;
        }
    }

  flatpak_system_helper_complete_configure_remote (object, invocation);

  return TRUE;
}

static gboolean
handle_configure (FlatpakSystemHelper   *object,
                  GDBusMethodInvocation *invocation,
                  guint                  arg_flags,
                  const gchar           *arg_key,
                  const gchar           *arg_value,
                  const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("Configure %u %s=%s %s", arg_flags, arg_key, arg_value, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if ((arg_flags & ~FLATPAK_HELPER_CONFIGURE_FLAGS_ALL) != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", (arg_flags & ~FLATPAK_HELPER_CONFIGURE_FLAGS_ALL));
      return TRUE;
    }

  /* We only support this for now */
  if (strcmp (arg_key, "languages") != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported key: %s", arg_key);
      return TRUE;
    }

  if ((arg_flags & FLATPAK_HELPER_CONFIGURE_FLAGS_UNSET) != 0)
    arg_value = NULL;

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_set_config (system, arg_key, arg_value, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_configure (object, invocation);

  return TRUE;
}

static gboolean
handle_update_remote (FlatpakSystemHelper   *object,
                      GDBusMethodInvocation *invocation,
                      guint                  arg_flags,
                      const gchar           *arg_remote,
                      const gchar           *arg_installation,
                      const gchar           *arg_summary_path,
                      const gchar           *arg_summary_sig_path)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;
  char *summary_data = NULL;
  gsize summary_size;
  g_autoptr(GBytes) summary_bytes = NULL;
  char *summary_sig_data = NULL;
  gsize summary_sig_size;
  g_autoptr(GBytes) summary_sig_bytes = NULL;
  g_autoptr(FlatpakRemoteState) state = NULL;

  g_debug ("UpdateRemote %u %s %s %s %s", arg_flags, arg_remote, arg_installation, arg_summary_path, arg_summary_sig_path);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (*arg_remote == 0 || strchr (arg_remote, '/') != NULL)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Invalid remote name: %s", arg_remote);
      return TRUE;
    }

  if ((arg_flags & ~FLATPAK_HELPER_UPDATE_REMOTE_FLAGS_ALL) != 0)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Unsupported flags enabled: 0x%x", (arg_flags & ~FLATPAK_HELPER_UPDATE_REMOTE_FLAGS_ALL));
      return TRUE;
    }

  if (!g_file_get_contents (arg_summary_path, &summary_data, &summary_size, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }
  summary_bytes = g_bytes_new_take (summary_data, summary_size);

  if (*arg_summary_sig_path != 0)
    {
      if (!g_file_get_contents (arg_summary_sig_path, &summary_sig_data, &summary_sig_size, &error))
        {
          g_dbus_method_invocation_return_gerror (invocation, error);
          return TRUE;
        }
      summary_sig_bytes = g_bytes_new_take (summary_sig_data, summary_sig_size);
    }

  state = flatpak_dir_get_remote_state_for_summary (system, arg_remote, summary_bytes, summary_sig_bytes, NULL, &error);
  if (state == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (summary_sig_bytes == NULL && state->collection_id == NULL)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "UpdateRemote requires a summary signature");
      return TRUE;
    }

  if (!flatpak_dir_update_remote_configuration_for_state (system, state, FALSE, NULL, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_update_remote (object, invocation);

  return TRUE;
}

static gboolean
handle_remove_local_ref (FlatpakSystemHelper   *object,
                         GDBusMethodInvocation *invocation,
                         const gchar           *arg_remote,
                         const gchar           *arg_ref,
                         const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("RemoveLocalRef %s %s %s", arg_remote, arg_ref, arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (*arg_remote == 0 || strchr (arg_remote, '/') != NULL)
    {
      g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                                             "Invalid remote name: %s", arg_remote);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_remove_ref (system, arg_remote, arg_ref, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_remove_local_ref (object, invocation);

  return TRUE;
}

static gboolean
handle_prune_local_repo (FlatpakSystemHelper   *object,
                         GDBusMethodInvocation *invocation,
                         const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("PruneLocalRepo %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_prune (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_prune_local_repo (object, invocation);

  return TRUE;
}


static gboolean
handle_ensure_repo (FlatpakSystemHelper   *object,
                    GDBusMethodInvocation *invocation,
                    const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("EnsureRepo %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_ensure_repo (object, invocation);

  return TRUE;
}

static gboolean
handle_run_triggers (FlatpakSystemHelper   *object,
                     GDBusMethodInvocation *invocation,
                     const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("RunTriggers %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_run_triggers (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_run_triggers (object, invocation);

  return TRUE;
}

static gboolean
handle_update_summary (FlatpakSystemHelper   *object,
                       GDBusMethodInvocation *invocation,
                       const gchar           *arg_installation)
{
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("UpdateSummary %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_ensure_repo (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  if (!flatpak_dir_update_summary (system, NULL, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  flatpak_system_helper_complete_update_summary (object, invocation);

  return TRUE;
}

static gboolean
handle_cancel_pull (FlatpakSystemHelper   *object,
                    GDBusMethodInvocation *invocation,
                    const gchar           *arg_installation,
                    const gchar           *arg_repo_path)
{
  g_autoptr(FlatpakSystemHelperOngoingPull) ongoing_pull = NULL;
  g_autoptr(FlatpakDir) system = NULL;
  g_autoptr(GError) error = NULL;

  g_debug ("CancelPull %s", arg_installation);

  system = dir_get_system (arg_installation, &error);
  if (system == NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      return TRUE;
    }

  ongoing_pull = get_ongoing_pull_by_dir (arg_repo_path);
  flatpak_system_helper_complete_cancel_pull (object, invocation);

  return TRUE;
}

static gboolean
flatpak_authorize_method_handler (GDBusInterfaceSkeleton *interface,
                                  GDBusMethodInvocation  *invocation,
                                  gpointer                user_data)
{
  const gchar *method_name = g_dbus_method_invocation_get_method_name (invocation);
  const gchar *sender = g_dbus_method_invocation_get_sender (invocation);
  GVariant *parameters = g_dbus_method_invocation_get_parameters (invocation);

  g_autoptr(AutoPolkitSubject) subject = polkit_system_bus_name_new (sender);
  g_autoptr(AutoPolkitDetails) details = polkit_details_new ();
  const gchar *action = NULL;
  gboolean authorized = FALSE;

  /* Ensure we don't idle exit */
  schedule_idle_callback ();

  if (on_session_bus)
    {
      /* This is test code, make sure it never runs with privileges */
      g_assert (geteuid () != 0);
      g_assert (getuid () != 0);
      g_assert (getegid () != 0);
      g_assert (getgid () != 0);
      authorized = TRUE;
    }
  else if (g_strcmp0 (method_name, "Deploy") == 0)
    {
      const char *ref, *origin;
      guint32 flags;
      gboolean is_update, is_app;

      g_variant_get_child (parameters, 1, "u", &flags);
      g_variant_get_child (parameters, 2, "&s", &ref);
      g_variant_get_child (parameters, 3, "&s", &origin);

      is_update = (flags & FLATPAK_HELPER_DEPLOY_FLAGS_UPDATE) != 0;
      is_app = g_str_has_prefix (ref, "app/");

      if (is_update)
        {
          if (is_app)
            action = "org.freedesktop.Flatpak.app-update";
          else
            action = "org.freedesktop.Flatpak.runtime-update";
        }
      else
        {
          if (is_app)
            action = "org.freedesktop.Flatpak.app-install";
          else
            action = "org.freedesktop.Flatpak.runtime-install";
        }

      polkit_details_insert (details, "origin", origin);
      polkit_details_insert (details, "ref", ref);
    }
  else if (g_strcmp0 (method_name, "DeployAppstream") == 0)
    {
      const char *arch, *origin;

      g_variant_get_child (parameters, 1, "&s", &origin);
      g_variant_get_child (parameters, 2, "&s", &arch);

      action = "org.freedesktop.Flatpak.appstream-update";

      polkit_details_insert (details, "origin", origin);
      polkit_details_insert (details, "arch", arch);
    }
  else if (g_strcmp0 (method_name, "InstallBundle") == 0)
    {
      const char *path;

      g_variant_get_child (parameters, 0, "^&ay", &path);

      action = "org.freedesktop.Flatpak.install-bundle";

      polkit_details_insert (details, "path", path);
    }
  else if (g_strcmp0 (method_name, "Uninstall") == 0)
    {
      const char *ref;
      gboolean is_app;

      g_variant_get_child (parameters, 1, "&s", &ref);

      is_app = g_str_has_prefix (ref, "app/");
      if (is_app)
        action = "org.freedesktop.Flatpak.app-uninstall";
      else
        action = "org.freedesktop.Flatpak.runtime-uninstall";

      polkit_details_insert (details, "ref", ref);
    }
  else if (g_strcmp0 (method_name, "ConfigureRemote") == 0)
    {
      const char *remote;

      g_variant_get_child (parameters, 1, "&s", &remote);

      action = "org.freedesktop.Flatpak.configure-remote";

      polkit_details_insert (details, "remote", remote);
    }
  else if (g_strcmp0 (method_name, "Configure") == 0)
    {
      const char *key;

      g_variant_get_child (parameters, 1, "&s", &key);

      action = "org.freedesktop.Flatpak.configure";

      polkit_details_insert (details, "key", key);
    }
  else if (g_strcmp0 (method_name, "UpdateRemote") == 0)
    {
      const char *remote;

      g_variant_get_child (parameters, 1, "&s", &remote);

      action = "org.freedesktop.Flatpak.update-remote";

      polkit_details_insert (details, "remote", remote);
    }
  else if (g_strcmp0 (method_name, "RemoveLocalRef") == 0 ||
           g_strcmp0 (method_name, "PruneLocalRepo") == 0 ||
           g_strcmp0 (method_name, "EnsureRepo") == 0 ||
           g_strcmp0 (method_name, "RunTriggers") == 0 ||
           g_strcmp0 (method_name, "UpdateSummary") == 0 ||
           g_strcmp0 (method_name, "GetChildRepoForPull") == 0 ||
           g_strcmp0 (method_name, "CancelPull"))
    {
      const char *remote;

      g_variant_get_child (parameters, 0, "&s", &remote);

      action = "org.freedesktop.Flatpak.modify-repo";

      polkit_details_insert (details, "remote", remote);
    }

  if (action)
    {
      g_autoptr(AutoPolkitAuthorizationResult) result = NULL;
      g_autoptr(GError) error = NULL;

      result = polkit_authority_check_authorization_sync (authority, subject,
                                                          action, details,
                                                          POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION,
                                                          NULL, &error);
      if (result == NULL)
        {
          g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                                 "Authorization error: %s", error->message);
          return FALSE;
        }

      authorized = polkit_authorization_result_get_is_authorized (result);
    }

  if (!authorized)
    {
      g_dbus_method_invocation_return_error (invocation,
                                             G_DBUS_ERROR,
                                             G_DBUS_ERROR_ACCESS_DENIED,
                                             "Flatpak system operation %s not allowed for user", method_name);
    }

  return authorized;
}

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  GError *error = NULL;

  g_debug ("Bus acquired, creating skeleton");

  helper = flatpak_system_helper_skeleton_new ();

  flatpak_system_helper_set_version (FLATPAK_SYSTEM_HELPER (helper), 1);

  g_object_set_data_full (G_OBJECT (helper), "track-alive", GINT_TO_POINTER (42), skeleton_died_cb);

  g_dbus_interface_skeleton_set_flags (G_DBUS_INTERFACE_SKELETON (helper),
                                       G_DBUS_INTERFACE_SKELETON_FLAGS_HANDLE_METHOD_INVOCATIONS_IN_THREAD);

  g_signal_connect (helper, "handle-deploy", G_CALLBACK (handle_deploy), NULL);
  g_signal_connect (helper, "handle-deploy-appstream", G_CALLBACK (handle_deploy_appstream), NULL);
  g_signal_connect (helper, "handle-uninstall", G_CALLBACK (handle_uninstall), NULL);
  g_signal_connect (helper, "handle-install-bundle", G_CALLBACK (handle_install_bundle), NULL);
  g_signal_connect (helper, "handle-configure-remote", G_CALLBACK (handle_configure_remote), NULL);
  g_signal_connect (helper, "handle-configure", G_CALLBACK (handle_configure), NULL);
  g_signal_connect (helper, "handle-update-remote", G_CALLBACK (handle_update_remote), NULL);
  g_signal_connect (helper, "handle-remove-local-ref", G_CALLBACK (handle_remove_local_ref), NULL);
  g_signal_connect (helper, "handle-prune-local-repo", G_CALLBACK (handle_prune_local_repo), NULL);
  g_signal_connect (helper, "handle-ensure-repo", G_CALLBACK (handle_ensure_repo), NULL);
  g_signal_connect (helper, "handle-run-triggers", G_CALLBACK (handle_run_triggers), NULL);
  g_signal_connect (helper, "handle-update-summary", G_CALLBACK (handle_update_summary), NULL);
  g_signal_connect (helper, "handle-get-child-repo-for-pull", G_CALLBACK (handle_get_child_repo_for_pull), NULL);
  g_signal_connect (helper, "handle-cancel-pull", G_CALLBACK (handle_cancel_pull), NULL);

  g_signal_connect (helper, "g-authorize-method",
                    G_CALLBACK (flatpak_authorize_method_handler),
                    NULL);

  if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (helper),
                                         connection,
                                         "/org/freedesktop/Flatpak/SystemHelper",
                                         &error))
    {
      g_warning ("error: %s", error->message);
      g_error_free (error);
    }
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_debug ("Name acquired");
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_debug ("Name lost");
  unref_skeleton_in_timeout ();
}

static void
binary_file_changed_cb (GFileMonitor     *file_monitor,
                        GFile            *file,
                        GFile            *other_file,
                        GFileMonitorEvent event_type,
                        gpointer          data)
{
  static gboolean got_it = FALSE;

  if (!got_it)
    {
      g_debug ("binary file changed");
      unref_skeleton_in_timeout ();
    }

  got_it = TRUE;
}

static void
message_handler (const gchar   *log_domain,
                 GLogLevelFlags log_level,
                 const gchar   *message,
                 gpointer       user_data)
{
  /* Make this look like normal console output */
  if (log_level & G_LOG_LEVEL_DEBUG)
    g_printerr ("F: %s\n", message);
  else
    g_printerr ("%s: %s\n", g_get_prgname (), message);
}

int
main (int    argc,
      char **argv)
{
  gchar exe_path[PATH_MAX + 1];
  ssize_t exe_path_len;
  gboolean replace;
  gboolean verbose;
  gboolean show_version;
  GBusNameOwnerFlags flags;
  GOptionContext *context;

  g_autoptr(GError) error = NULL;
  const GOptionEntry options[] = {
    { "replace", 'r', 0, G_OPTION_ARG_NONE, &replace,  "Replace old daemon.", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,  "Enable debug output.", NULL },
    { "session", 0, 0, G_OPTION_ARG_NONE, &on_session_bus,  "Run in session, not system scope (for tests).", NULL },
    { "no-idle-exit", 0, 0, G_OPTION_ARG_NONE, &no_idle_exit,  "Don't exit when idle.", NULL },
    { "version", 0, 0, G_OPTION_ARG_NONE, &show_version, "Show program version.", NULL},
    { NULL }
  };

  setlocale (LC_ALL, "");

  g_setenv ("GIO_USE_VFS", "local", TRUE);

  g_set_prgname (argv[0]);

  g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, message_handler, NULL);

  context = g_option_context_new ("");

  replace = FALSE;
  verbose = FALSE;
  show_version = FALSE;

  g_option_context_set_summary (context, "Flatpak system helper");
  g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s: %s", g_get_application_name (), error->message);
      g_printerr ("\n");
      g_printerr ("Try \"%s --help\" for more information.",
                  g_get_prgname ());
      g_printerr ("\n");
      g_option_context_free (context);
      return 1;
    }

  if (show_version)
    {
      g_print (PACKAGE_STRING "\n");
      return 0;
    }

  if (verbose)
    g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, message_handler, NULL);

  if (!on_session_bus)
    {
      authority = polkit_authority_get_sync (NULL, &error);
      if (authority == NULL)
        {
          g_printerr ("Can't get polkit authority: %s\n", error->message);
          return 1;
        }
    }

  exe_path_len = readlink ("/proc/self/exe", exe_path, sizeof (exe_path) - 1);
  if (exe_path_len > 0)
    {
      exe_path[exe_path_len] = 0;
      GFileMonitor *monitor;
      g_autoptr(GFile) exe = NULL;
      g_autoptr(GError) local_error = NULL;

      exe = g_file_new_for_path (exe_path);
      monitor =  g_file_monitor_file (exe,
                                      G_FILE_MONITOR_NONE,
                                      NULL,
                                      &local_error);
      if (monitor == NULL)
        g_warning ("Failed to set watch on %s: %s", exe_path, error->message);
      else
        g_signal_connect (monitor, "changed",
                          G_CALLBACK (binary_file_changed_cb), NULL);
    }

  flags = G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT;
  if (replace)
    flags |= G_BUS_NAME_OWNER_FLAGS_REPLACE;

  name_owner_id = g_bus_own_name (on_session_bus ? G_BUS_TYPE_SESSION  : G_BUS_TYPE_SYSTEM,
                                  "org.freedesktop.Flatpak.SystemHelper",
                                  flags,
                                  on_bus_acquired,
                                  on_name_acquired,
                                  on_name_lost,
                                  NULL,
                                  NULL);

  ongoing_pulls = g_ptr_array_new ();

  /* Ensure we don't idle exit */
  schedule_idle_callback ();

  main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (main_loop);

  g_clear_pointer (&ongoing_pulls, g_ptr_array_unref);

  return 0;
}
