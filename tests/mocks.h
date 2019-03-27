/* tests/mocks.h -- Riemann C client library
 * Copyright (C) 2014  Gergely Nagy <algernon@madhouse-project.org>
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

#ifndef __MADHOUSE_RIEMANN_TESTS_MOCKS_H__
#define __MADHOUSE_RIEMANN_TESTS_MOCKS_H__ 1

#define UNUSED __attribute__((unused))

#include <sys/types.h>
#include <sys/socket.h>

#define make_mock(name, retval, ...)                \
  static retval (*mock_##name) ();                  \
  static retval (*real_##name) ();                  \
  retval name (__VA_ARGS__) __attribute__((weak));  \
  retval name (__VA_ARGS__)

#define mock(name, func) mock_##name = func
#define restore(name) mock_##name = real_##name

#define STUB(name, ...)                         \
  if (!real_##name)                             \
    real_##name = dlsym (RTLD_NEXT, #name);     \
  if (!mock_##name)                             \
    mock_##name = real_##name;                  \
                                                \
  return mock_##name (__VA_ARGS__)

make_mock (socket, int, int domain, int type, int protocol)
{
  STUB (socket, domain, type, protocol);
}

make_mock (send, ssize_t, int sockfd, const void *buf, size_t len,
           int flags)
{
  STUB (send, sockfd, buf, len, flags);
}

make_mock (sendto, ssize_t, int sockfd, const void *buf, size_t len,
           int flags, const struct sockaddr *dest_addr,
           socklen_t addrlen)
{
  STUB (sendto, sockfd, buf, len, flags, dest_addr, addrlen);
}

make_mock (recv, ssize_t, int sockfd, void *buf, size_t len, int flags)
{
  STUB (recv, sockfd, buf, len, flags);
}

static int mock_enosys_int_always_fail () UNUSED;
static ssize_t mock_enosys_ssize_t_always_fail () UNUSED;

#endif
