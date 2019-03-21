#include <errno.h>

#include <riemann/client.h>
#include <check.h>

#include <platform.h>

START_TEST (test_riemann_c_client_library)
{
  ck_assert_str_eq (riemann_client_version (), PACKAGE_VERSION);
  ck_assert_str_eq (riemann_client_version_string (), PACKAGE_STRING);
}
END_TEST

static TCase *
test_riemann_library (void)
{
  TCase *test_library;

  test_library = tcase_create ("Core");
  tcase_add_test (test_library, test_riemann_c_client_library);

  return test_library;
}
