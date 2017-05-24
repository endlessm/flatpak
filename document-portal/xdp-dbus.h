/*
 * Generated by gdbus-codegen 2.53.2. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef ____DOCUMENT_PORTAL_XDP_DBUS_H__
#define ____DOCUMENT_PORTAL_XDP_DBUS_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.freedesktop.portal.Documents */

#define XDP_DBUS_TYPE_DOCUMENTS (xdp_dbus_documents_get_type ())
#define XDP_DBUS_DOCUMENTS(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDP_DBUS_TYPE_DOCUMENTS, XdpDbusDocuments))
#define XDP_DBUS_IS_DOCUMENTS(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDP_DBUS_TYPE_DOCUMENTS))
#define XDP_DBUS_DOCUMENTS_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), XDP_DBUS_TYPE_DOCUMENTS, XdpDbusDocumentsIface))

struct _XdpDbusDocuments;
typedef struct _XdpDbusDocuments XdpDbusDocuments;
typedef struct _XdpDbusDocumentsIface XdpDbusDocumentsIface;

struct _XdpDbusDocumentsIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_add) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list,
    GVariant *arg_o_path_fd,
    gboolean arg_reuse_existing,
    gboolean arg_persistent);

  gboolean (*handle_add_full) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list,
    GVariant *arg_o_path_fds,
    guint arg_flags,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions);

  gboolean (*handle_add_named) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_o_path_parent_fd,
    const gchar *arg_filename,
    gboolean arg_reuse_existing,
    gboolean arg_persistent);

  gboolean (*handle_delete) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_doc_id);

  gboolean (*handle_get_mount_point) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_grant_permissions) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions);

  gboolean (*handle_info) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_doc_id);

  gboolean (*handle_list) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_app_id);

  gboolean (*handle_lookup) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_filename);

  gboolean (*handle_revoke_permissions) (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions);

  guint  (*get_version) (XdpDbusDocuments *object);

};

GType xdp_dbus_documents_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *xdp_dbus_documents_interface_info (void);
guint xdp_dbus_documents_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void xdp_dbus_documents_complete_get_mount_point (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *path);

void xdp_dbus_documents_complete_add (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list,
    const gchar *doc_id);

void xdp_dbus_documents_complete_add_named (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *doc_id);

void xdp_dbus_documents_complete_add_full (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GUnixFDList *fd_list,
    const gchar *const *doc_ids,
    GVariant *extra_out);

void xdp_dbus_documents_complete_grant_permissions (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation);

void xdp_dbus_documents_complete_revoke_permissions (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation);

void xdp_dbus_documents_complete_delete (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation);

void xdp_dbus_documents_complete_lookup (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *doc_id);

void xdp_dbus_documents_complete_info (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    const gchar *path,
    GVariant *apps);

void xdp_dbus_documents_complete_list (
    XdpDbusDocuments *object,
    GDBusMethodInvocation *invocation,
    GVariant *docs);



/* D-Bus method calls: */
void xdp_dbus_documents_call_get_mount_point (
    XdpDbusDocuments *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_get_mount_point_finish (
    XdpDbusDocuments *proxy,
    gchar **out_path,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_get_mount_point_sync (
    XdpDbusDocuments *proxy,
    gchar **out_path,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_add (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_fd,
    gboolean arg_reuse_existing,
    gboolean arg_persistent,
    GUnixFDList *fd_list,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_add_finish (
    XdpDbusDocuments *proxy,
    gchar **out_doc_id,
    GUnixFDList **out_fd_list,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_add_sync (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_fd,
    gboolean arg_reuse_existing,
    gboolean arg_persistent,
    GUnixFDList  *fd_list,
    gchar **out_doc_id,
    GUnixFDList **out_fd_list,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_add_named (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_parent_fd,
    const gchar *arg_filename,
    gboolean arg_reuse_existing,
    gboolean arg_persistent,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_add_named_finish (
    XdpDbusDocuments *proxy,
    gchar **out_doc_id,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_add_named_sync (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_parent_fd,
    const gchar *arg_filename,
    gboolean arg_reuse_existing,
    gboolean arg_persistent,
    gchar **out_doc_id,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_add_full (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_fds,
    guint arg_flags,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GUnixFDList *fd_list,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_add_full_finish (
    XdpDbusDocuments *proxy,
    gchar ***out_doc_ids,
    GVariant **out_extra_out,
    GUnixFDList **out_fd_list,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_add_full_sync (
    XdpDbusDocuments *proxy,
    GVariant *arg_o_path_fds,
    guint arg_flags,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GUnixFDList  *fd_list,
    gchar ***out_doc_ids,
    GVariant **out_extra_out,
    GUnixFDList **out_fd_list,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_grant_permissions (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_grant_permissions_finish (
    XdpDbusDocuments *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_grant_permissions_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_revoke_permissions (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_revoke_permissions_finish (
    XdpDbusDocuments *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_revoke_permissions_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    const gchar *arg_app_id,
    const gchar *const *arg_permissions,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_delete (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_delete_finish (
    XdpDbusDocuments *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_delete_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_lookup (
    XdpDbusDocuments *proxy,
    const gchar *arg_filename,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_lookup_finish (
    XdpDbusDocuments *proxy,
    gchar **out_doc_id,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_lookup_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_filename,
    gchar **out_doc_id,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_info (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_info_finish (
    XdpDbusDocuments *proxy,
    gchar **out_path,
    GVariant **out_apps,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_info_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_doc_id,
    gchar **out_path,
    GVariant **out_apps,
    GCancellable *cancellable,
    GError **error);

void xdp_dbus_documents_call_list (
    XdpDbusDocuments *proxy,
    const gchar *arg_app_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdp_dbus_documents_call_list_finish (
    XdpDbusDocuments *proxy,
    GVariant **out_docs,
    GAsyncResult *res,
    GError **error);

gboolean xdp_dbus_documents_call_list_sync (
    XdpDbusDocuments *proxy,
    const gchar *arg_app_id,
    GVariant **out_docs,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
guint xdp_dbus_documents_get_version (XdpDbusDocuments *object);
void xdp_dbus_documents_set_version (XdpDbusDocuments *object, guint value);


/* ---- */

#define XDP_DBUS_TYPE_DOCUMENTS_PROXY (xdp_dbus_documents_proxy_get_type ())
#define XDP_DBUS_DOCUMENTS_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDP_DBUS_TYPE_DOCUMENTS_PROXY, XdpDbusDocumentsProxy))
#define XDP_DBUS_DOCUMENTS_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), XDP_DBUS_TYPE_DOCUMENTS_PROXY, XdpDbusDocumentsProxyClass))
#define XDP_DBUS_DOCUMENTS_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), XDP_DBUS_TYPE_DOCUMENTS_PROXY, XdpDbusDocumentsProxyClass))
#define XDP_DBUS_IS_DOCUMENTS_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDP_DBUS_TYPE_DOCUMENTS_PROXY))
#define XDP_DBUS_IS_DOCUMENTS_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), XDP_DBUS_TYPE_DOCUMENTS_PROXY))

typedef struct _XdpDbusDocumentsProxy XdpDbusDocumentsProxy;
typedef struct _XdpDbusDocumentsProxyClass XdpDbusDocumentsProxyClass;
typedef struct _XdpDbusDocumentsProxyPrivate XdpDbusDocumentsProxyPrivate;

struct _XdpDbusDocumentsProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  XdpDbusDocumentsProxyPrivate *priv;
};

struct _XdpDbusDocumentsProxyClass
{
  GDBusProxyClass parent_class;
};

GType xdp_dbus_documents_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdpDbusDocumentsProxy, g_object_unref)
#endif

void xdp_dbus_documents_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
XdpDbusDocuments *xdp_dbus_documents_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
XdpDbusDocuments *xdp_dbus_documents_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void xdp_dbus_documents_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
XdpDbusDocuments *xdp_dbus_documents_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
XdpDbusDocuments *xdp_dbus_documents_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define XDP_DBUS_TYPE_DOCUMENTS_SKELETON (xdp_dbus_documents_skeleton_get_type ())
#define XDP_DBUS_DOCUMENTS_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDP_DBUS_TYPE_DOCUMENTS_SKELETON, XdpDbusDocumentsSkeleton))
#define XDP_DBUS_DOCUMENTS_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), XDP_DBUS_TYPE_DOCUMENTS_SKELETON, XdpDbusDocumentsSkeletonClass))
#define XDP_DBUS_DOCUMENTS_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), XDP_DBUS_TYPE_DOCUMENTS_SKELETON, XdpDbusDocumentsSkeletonClass))
#define XDP_DBUS_IS_DOCUMENTS_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDP_DBUS_TYPE_DOCUMENTS_SKELETON))
#define XDP_DBUS_IS_DOCUMENTS_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), XDP_DBUS_TYPE_DOCUMENTS_SKELETON))

typedef struct _XdpDbusDocumentsSkeleton XdpDbusDocumentsSkeleton;
typedef struct _XdpDbusDocumentsSkeletonClass XdpDbusDocumentsSkeletonClass;
typedef struct _XdpDbusDocumentsSkeletonPrivate XdpDbusDocumentsSkeletonPrivate;

struct _XdpDbusDocumentsSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  XdpDbusDocumentsSkeletonPrivate *priv;
};

struct _XdpDbusDocumentsSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType xdp_dbus_documents_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdpDbusDocumentsSkeleton, g_object_unref)
#endif

XdpDbusDocuments *xdp_dbus_documents_skeleton_new (void);


G_END_DECLS

#endif /* ____DOCUMENT_PORTAL_XDP_DBUS_H__ */
