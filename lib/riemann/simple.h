/* riemann/simple.h -- Riemann C client library
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

#ifndef __MADHOUSE_RIEMANN_SIMPLE_H__
#define __MADHOUSE_RIEMANN_SIMPLE_H__ 1

#include <riemann/riemann-client.h>

int riemann_send (riemann_client_t *client,
                  riemann_event_field_t field, ...);
riemann_message_t *riemann_query (riemann_client_t *client,
                                  const char *query);

#endif
