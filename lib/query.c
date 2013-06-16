/* riemann/query.c -- Riemann C client library
 * Copyright (C) 2013  Gergely Nagy <algernon@madhouse-project.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <riemann/query.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

riemann_query_t *
riemann_query_new (const char *string)
{
  riemann_query_t *query;

  query = malloc (sizeof (riemann_query_t));
  query__init (query);

  if (!string)
    return query;

  if (riemann_query_set_string (query, string) != 0)
    {
      riemann_query_free (query);
      return NULL;
    }

  return query;
}

void
riemann_query_free (riemann_query_t *query)
{
  if (!query)
    return;

  query__free_unpacked (query, NULL);
}

int
riemann_query_set_string (riemann_query_t *query, const char *string)
{
  if (!query || !string)
    return -EINVAL;

  if (query->string)
    free (query->string);

  query->string = strdup (string);

  return 0;
}
