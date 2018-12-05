/*
 * Copyright © 2018 Endless Mobile, Inc.
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
 *       Philip Withnall <withnall@endlessm.com>
 */

#include "config.h"

#include <glib.h>
#include <gio/gio.h>
#include <appstream-glib.h>
#include <libeos-parental-controls/app-filter.h>

#include "flatpak-parental-controls-private.h"

/* Convert an #EpcAppFilterOarsValue to an #AsContentRatingValue. This is
 * actually a trivial cast, since the types are defined the same; but throw in
 * a static assertion to be sure. */
static AsContentRatingValue
convert_app_filter_oars_value (EpcAppFilterOarsValue filter_value)
{
  G_STATIC_ASSERT (AS_CONTENT_RATING_VALUE_LAST == EPC_APP_FILTER_OARS_VALUE_INTENSE + 1);

  return (AsContentRatingValue) filter_value;
}

/**
 * flatpak_appstream_get_latest_content_rating:
 * @app: an #AsApp
 *
 * Get the #AsContentRating from @app with the latest OARS standards version.
 * If no suitable #AsContentRating can be found, %NULL is returned.
 *
 * Returns: (nullable) (transfer none): latest OARS content rating from @app,
 *    or %NULL if none was found
 */
AsContentRating *
flatpak_appstream_get_latest_content_rating (AsApp *app)
{
  AsContentRating *rating = NULL;

  rating = as_app_get_content_rating (app, "oars-1.1");
  if (rating != NULL)
    return rating;

  rating = as_app_get_content_rating (app, "oars-1.0");
  if (rating != NULL)
    return rating;

  return NULL;
}

/**
 * flatpak_appstream_check_rating:
 * @rating: (nullable): OARS ratings for the app, or %NULL if none are known
 * @filter: user’s parental controls settings
 *
 * Check whether the OARS rating in @rating is as, or less, extreme than the
 * user’s preferences in @filter. If so (i.e. if the app is suitable for this
 * user to use), return %TRUE; otherwise return %FALSE.
 *
 * @rating may be %NULL if no OARS ratings are provided for the app. If so,
 * we have to assume the most restrictive ratings.
 *
 * Returns: %TRUE if the app is safe to install, %FALSE otherwise
 */
gboolean
flatpak_appstream_check_rating (AsContentRating *rating,
                                EpcAppFilter    *filter)
{
  g_autofree const gchar **oars_sections = epc_app_filter_get_oars_sections (filter);

  for (gsize i = 0; oars_sections[i] != NULL; i++)
    {
      AsContentRatingValue rating_value;
      EpcAppFilterOarsValue filter_value = epc_app_filter_get_oars_value (filter,
                                                                          oars_sections[i]);

      if (rating != NULL)
        rating_value = as_content_rating_get_value (rating, oars_sections[i]);
      else
        rating_value = AS_CONTENT_RATING_VALUE_INTENSE;

      if (rating_value == AS_CONTENT_RATING_VALUE_UNKNOWN ||
          filter_value == EPC_APP_FILTER_OARS_VALUE_UNKNOWN)
        continue;
      else if (convert_app_filter_oars_value (filter_value) < rating_value)
        return FALSE;
    }

  return TRUE;
}
