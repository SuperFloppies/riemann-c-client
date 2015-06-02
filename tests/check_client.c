#include <riemann/client.h>
#include "riemann/platform.h"
#include "riemann/_private.h"
#include "mocks.h"

#if HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif

START_TEST (test_riemann_client_new)
{
  riemann_client_t *client;

  client = riemann_client_new ();
  ck_assert (client != NULL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_free)
{
  errno = 0;
  riemann_client_free (NULL);
  ck_assert_errno (-errno, EINVAL);
}
END_TEST

START_TEST (test_riemann_client_connect)
{
  riemann_client_t *client;

  client = riemann_client_new ();

  ck_assert_errno (riemann_client_connect (NULL, RIEMANN_CLIENT_TCP,
                                           "127.0.0.1", 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_NONE,
                                           "127.0.0.1", 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                           NULL, 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                           "127.0.0.1", -1), ERANGE);

  if (network_tests_enabled ())
    {
      ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                               "127.0.0.1", 5557), ECONNREFUSED);

      ck_assert (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                         "127.0.0.1", 5555) == 0);
      ck_assert_errno (riemann_client_disconnect (client), 0);

      ck_assert (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                         "127.0.0.1", 5555,
                                         RIEMANN_CLIENT_OPTION_NONE) == 0);
      ck_assert_errno (riemann_client_disconnect (client), 0);

      ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                               "non-existent.example.com", 5555),
                       EADDRNOTAVAIL);

      mock (socket, mock_enosys_int_always_fail);
      ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                               "127.0.0.1", 5555),
                       ENOSYS);
      restore (socket);

      /** TLS tests **/
#if HAVE_GNUTLS
      ck_assert_errno
        (riemann_client_connect
         (client, RIEMANN_CLIENT_TLS,
          "127.0.0.1", 5554,
          RIEMANN_CLIENT_OPTION_NONE),
         EINVAL);

      ck_assert_errno
        (riemann_client_connect
         (client, RIEMANN_CLIENT_TLS,
          "127.0.0.1", 5554,
          256,
          RIEMANN_CLIENT_OPTION_NONE),
         EINVAL);

      ck_assert_errno
        (riemann_client_connect
         (client, RIEMANN_CLIENT_TLS,
          "127.0.0.1", 5554,
          RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
          RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
          RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
          RIEMANN_CLIENT_OPTION_NONE),
         0);
      riemann_client_disconnect (client);

      ck_assert_errno
        (riemann_client_connect
         (client, RIEMANN_CLIENT_TLS,
          "127.0.0.1", 5554,
          RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert-invalid.pem",
          RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
          RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
          RIEMANN_CLIENT_OPTION_NONE),
         EPROTO);

#if GNUTLS_VERSION_MAJOR > 2
      ck_assert_errno
        (riemann_client_connect
         (client, RIEMANN_CLIENT_TLS,
          "127.0.0.1", 5555,
          RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
          RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
          RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
          RIEMANN_CLIENT_OPTION_TLS_HANDSHAKE_TIMEOUT, 1000,
          RIEMANN_CLIENT_OPTION_NONE),
         EPROTO);
#endif

#endif
    }

  ck_assert (client != NULL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_get_fd)
{
  ck_assert_errno (riemann_client_get_fd (NULL), EINVAL);

  if (network_tests_enabled ())
    {
      riemann_client_t *client;

      client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
      ck_assert (riemann_client_get_fd (client) != 0);
      riemann_client_free (client);
    }
}
END_TEST

make_mock (setsockopt, int,
           int sockfd, int level, int optname,
           const void *optval, socklen_t optlen)
{
  STUB (setsockopt, sockfd, level, optname, optval, optlen);
}

static int
_mock_setsockopt (int sockfd __attribute__((unused)),
                  int level __attribute__((unused)),
                  int optname,
                  const void *optval __attribute__((unused)),
                  socklen_t optlen __attribute__((unused)))
{
  if (optname == SO_RCVTIMEO)
    {
      errno = ENOSYS;
      return -1;
    }

  return 0;
}

START_TEST (test_riemann_client_set_timeout)
{
  struct timeval timeout;
  riemann_client_t *client;

  timeout.tv_sec = 5;
  timeout.tv_usec = 42;

  ck_assert_errno (riemann_client_set_timeout (NULL, NULL), EINVAL);
  ck_assert_errno (riemann_client_set_timeout (NULL, &timeout), EINVAL);

  client = riemann_client_new ();
  ck_assert_errno (riemann_client_set_timeout (client, &timeout), EINVAL);
  riemann_client_free (client);

  if (network_tests_enabled ())
    {
      int fd;

      client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
      ck_assert_errno (riemann_client_set_timeout (client, NULL), EINVAL);
      ck_assert_errno (riemann_client_set_timeout (client, &timeout), 0);

      fd = client->sock;
      client->sock = client->sock + 10;

      ck_assert_errno (riemann_client_set_timeout (client, &timeout), EBADF);

      mock (setsockopt, _mock_setsockopt);
      ck_assert_errno (riemann_client_set_timeout (client, &timeout), ENOSYS);
      restore (setsockopt);

      client->sock = fd;

      riemann_client_free (client);
    }
}
END_TEST

START_TEST (test_riemann_client_disconnect)
{
  ck_assert_errno (riemann_client_disconnect (NULL), ENOTCONN);

  if (network_tests_enabled ())
    {
      riemann_client_t *client;

      client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
      client->sock++;

      ck_assert_errno (riemann_client_disconnect (client), EBADF);
      client->sock--;
      riemann_client_free (client);
    }
}
END_TEST

START_TEST (test_riemann_client_create)
{
  riemann_client_t *client;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5557);
  ck_assert (client == NULL);
  ck_assert_errno (-errno, ECONNREFUSED);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  ck_assert (client != NULL);
  ck_assert_errno (riemann_client_disconnect (client), 0);
  ck_assert (client != NULL);
  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555,
                                  RIEMANN_CLIENT_OPTION_NONE);
  ck_assert (client != NULL);
  ck_assert_errno (riemann_client_disconnect (client), 0);
  ck_assert (client != NULL);
  riemann_client_free (client);

#if HAVE_GNUTLS
  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  ck_assert (client != NULL);
  ck_assert_errno (riemann_client_disconnect (client), 0);
  ck_assert (client != NULL);
  riemann_client_free (client);
#else
  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  ck_assert (client == NULL);
  ck_assert_errno (-errno, ENOTSUP);
#endif
}
END_TEST

make_mock (riemann_message_to_buffer, uint8_t *,
           riemann_message_t *message, size_t *len)
{
  STUB (riemann_message_to_buffer, message, len);
}

static uint8_t *
_mock_message_to_buffer ()
{
  errno = EPROTO;
  return NULL;
}

START_TEST (test_riemann_client_send_message)
{
  riemann_client_t *client, *client_fresh;
  riemann_message_t *message;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  ck_assert_errno (riemann_client_send_message (NULL, message), ENOTCONN);
  ck_assert_errno (riemann_client_send_message (client, NULL), EINVAL);

  client_fresh = riemann_client_new ();
  ck_assert_errno (riemann_client_send_message (client_fresh, message), ENOTCONN);
  riemann_client_free (client_fresh);

  mock (riemann_message_to_buffer, _mock_message_to_buffer);
  ck_assert_errno (riemann_client_send_message (client, message),
                   EPROTO);
  restore (riemann_message_to_buffer);

  ck_assert_errno (riemann_client_send_message (client, message), 0);

  mock (send, mock_enosys_ssize_t_always_fail);
  ck_assert_errno (riemann_client_send_message (client, message), ENOSYS);
  restore (send);

  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_UDP, "127.0.0.1", 5555);

  mock (riemann_message_to_buffer, _mock_message_to_buffer);
  ck_assert_errno (riemann_client_send_message (client, message),
                   EPROTO);
  restore (riemann_message_to_buffer);

  ck_assert_errno (riemann_client_send_message (client, message), 0);

  mock (sendto, mock_enosys_ssize_t_always_fail);
  ck_assert_errno (riemann_client_send_message (client, message), ENOSYS);
  restore (sendto);

  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST

static ssize_t
_mock_recv_message_part (int sockfd, void *buf, size_t len, int flags)
{
  static int counter;

  counter++;
  if (counter % 2 == 0)
    {
      errno = ENOSYS;
      return -1;
    }

  return real_recv (sockfd, buf, len, flags);
}

static ssize_t
_mock_recv_message_garbage (int sockfd, void *buf, size_t len, int flags)
{
  static int counter;
  ssize_t res;

  counter++;
  res = real_recv (sockfd, buf, len, flags);

  if (counter % 2 == 0)
    memset (buf, 128, len);

  return res;
}

START_TEST (test_riemann_client_recv_message)
{
  riemann_client_t *client, *client_fresh;
  riemann_message_t *message, *response = NULL;

  errno = 0;
  ck_assert (riemann_client_recv_message (NULL) == NULL);
  ck_assert_errno (-errno, ENOTCONN);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  client_fresh = riemann_client_new ();
  ck_assert (riemann_client_recv_message (client_fresh) == NULL);
  ck_assert_errno (-errno, ENOTCONN);
  riemann_client_free (client_fresh);

  riemann_client_send_message (client, message);
  ck_assert ((response = riemann_client_recv_message (client)) != NULL);
  ck_assert_int_eq (response->ok, 1);
  riemann_message_free (response);
  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  riemann_client_send_message (client, message);
  mock (recv, mock_enosys_ssize_t_always_fail);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, ENOSYS);
  restore (recv);
  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  riemann_client_send_message (client, message);
  mock (recv, _mock_recv_message_part);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, ENOSYS);
  restore (recv);
  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  riemann_client_send_message (client, message);
  mock (recv, _mock_recv_message_garbage);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, EPROTO);
  restore (recv);
  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_UDP, "127.0.0.1", 5555);

  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, ENOTSUP);

  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_client_send_message_oneshot)
{
  riemann_client_t *client, *client_fresh;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);
  ck_assert_errno (riemann_client_send_message_oneshot
                   (NULL, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), ENOTCONN);
  ck_assert_errno (riemann_client_send_message (client, NULL), EINVAL);

  client_fresh = riemann_client_new ();
  ck_assert_errno (riemann_client_send_message_oneshot
                   (client_fresh, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), ENOTCONN);
  riemann_client_free (client_fresh);

  ck_assert_errno (riemann_client_send_message_oneshot
                   (client, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), 0);

  riemann_client_free (client);
}
END_TEST

#if HAVE_GNUTLS
make_mock (gnutls_record_send, ssize_t, gnutls_session_t session,
           const void *data, size_t len)
{
  STUB (gnutls_record_send, session, data, len);
}

make_mock (gnutls_record_recv, ssize_t, gnutls_session_t session,
           void *buf, size_t len)
{
  STUB (gnutls_record_recv, session, buf, len);
}

static ssize_t
_mock_gnutls_record_recv_message_part (gnutls_session_t session,
                                       void *buf, size_t len)
{
  static int counter;

  counter++;
  if (counter % 2 == 0)
    {
      errno = ENOSYS;
      return -1;
    }

  return real_gnutls_record_recv (session, buf, len);
}

static ssize_t
_mock_gnutls_record_recv_message_garbage (gnutls_session_t session,
                                          void *buf, size_t len)
{
  static int counter;
  ssize_t res;

  counter++;
  res = real_gnutls_record_recv (session, buf, len);

  if (counter % 2 == 0)
    memset (buf, 128, len);

  return res;
}

START_TEST (test_riemann_client_send_message_tls)
{
  riemann_client_t *client;
  riemann_message_t *message;

  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);

  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  ck_assert_errno (riemann_client_send_message (NULL, message), ENOTCONN);
  ck_assert_errno (riemann_client_send_message (client, NULL), EINVAL);

  mock (riemann_message_to_buffer, _mock_message_to_buffer);
  ck_assert_errno (riemann_client_send_message (client, message),
                   EPROTO);
  restore (riemann_message_to_buffer);

  ck_assert_errno (riemann_client_send_message (client, message), 0);

  mock (gnutls_record_send, mock_enosys_ssize_t_always_fail);
  ck_assert_errno (riemann_client_send_message (client, message), EPROTO);
  restore (gnutls_record_send);

  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_client_recv_message_tls)
{
  riemann_client_t *client;
  riemann_message_t *message, *response = NULL;

  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  riemann_client_send_message (client, message);
  ck_assert ((response = riemann_client_recv_message (client)) != NULL);
  ck_assert_int_eq (response->ok, 1);
  riemann_message_free (response);
  riemann_client_free (client);

  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  riemann_client_send_message (client, message);
  mock (gnutls_record_recv, mock_enosys_ssize_t_always_fail);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, EPROTO);
  restore (gnutls_record_recv);
  riemann_client_free (client);

  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  riemann_client_send_message (client, message);
  mock (gnutls_record_recv, _mock_gnutls_record_recv_message_part);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, EPROTO);
  restore (recv);
  riemann_client_free (client);

  client = riemann_client_create
    (RIEMANN_CLIENT_TLS,
     "127.0.0.1", 5554,
     RIEMANN_CLIENT_OPTION_TLS_CA_FILE, "tests/data/cacert.pem",
     RIEMANN_CLIENT_OPTION_TLS_CERT_FILE, "tests/data/client.crt",
     RIEMANN_CLIENT_OPTION_TLS_KEY_FILE, "tests/data/client.key",
     RIEMANN_CLIENT_OPTION_NONE);
  riemann_client_send_message (client, message);
  mock (gnutls_record_recv, _mock_gnutls_record_recv_message_garbage);
  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, EPROTO);
  restore (gnutls_record_recv);
  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST
#endif

static TCase *
test_riemann_client (void)
{
  TCase *test_client;

  test_client = tcase_create ("Client");
  tcase_add_test (test_client, test_riemann_client_new);
  tcase_add_test (test_client, test_riemann_client_free);
  tcase_add_test (test_client, test_riemann_client_connect);
  tcase_add_test (test_client, test_riemann_client_disconnect);
  tcase_add_test (test_client, test_riemann_client_get_fd);
  tcase_add_test (test_client, test_riemann_client_set_timeout);

  if (network_tests_enabled ())
    {
      tcase_add_test (test_client, test_riemann_client_create);
      tcase_add_test (test_client, test_riemann_client_send_message);
      tcase_add_test (test_client, test_riemann_client_send_message_oneshot);
      tcase_add_test (test_client, test_riemann_client_recv_message);

#if HAVE_GNUTLS
      tcase_add_test (test_client, test_riemann_client_send_message_tls);
      tcase_add_test (test_client, test_riemann_client_recv_message_tls);
#endif
    }

  return test_client;
}
