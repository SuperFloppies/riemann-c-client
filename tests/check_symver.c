#include <check.h>
#include <errno.h>
#include <stdlib.h>
#include <riemann/client.h>

#include <config.h>
#include "tests.h"

#ifdef HAVE_VERSIONING
START_TEST (test_riemann_client_connect_1_0)
{
  riemann_client_t *client;

  client = riemann_client_new ();

  asm (".symver riemann_client_connect, riemann_client_connect@RIEMANN_C_1.0");

  ck_assert_errno (riemann_client_connect (NULL, RIEMANN_CLIENT_TCP,
                                           "127.0.0.1", 5555), EINVAL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_create_1_0)
{
  riemann_client_t *client;

  asm (".symver riemann_client_create, riemann_client_create@RIEMANN_C_1.0");

  client = riemann_client_create (RIEMANN_CLIENT_NONE,
                                  "127.0.0.1", 5555);

  ck_assert (client == NULL);
}
END_TEST
#endif

static TCase *
test_riemann_symbol_versioning (void)
{
  TCase *test_symver;

  test_symver = tcase_create ("Symbol versioning");

#ifdef HAVE_VERSIONING
  tcase_add_test (test_symver, test_riemann_client_connect_1_0);
  tcase_add_test (test_symver, test_riemann_client_create_1_0);
#endif

  return test_symver;
}

int
main (void)
{
  Suite *suite;
  SRunner *runner;

  int nfailed;

  suite = suite_create ("Riemann C client library symbol versioning tests");

  suite_add_tcase (suite, test_riemann_symbol_versioning ());

  runner = srunner_create (suite);

  srunner_run_all (runner, CK_ENV);
  nfailed = srunner_ntests_failed (runner);
  srunner_free (runner);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
