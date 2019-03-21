#include <riemann/attribute.h>

#include <check.h>
#include <errno.h>

START_TEST (test_riemann_attribute_new_and_free)
{
  riemann_attribute_t *attrib;

  ck_assert ((attrib = riemann_attribute_new ()) != NULL);

  riemann_attribute_free (attrib);

  errno = 0;
  riemann_attribute_free (NULL);
  ck_assert_errno (-errno, EINVAL);
}
END_TEST

START_TEST (test_riemann_attribute_set)
{
  riemann_attribute_t *attrib;

  attrib = riemann_attribute_new ();

  ck_assert_errno (riemann_attribute_set_key (NULL, "foobar"), EINVAL);
  ck_assert_errno (riemann_attribute_set_key (attrib, NULL), EINVAL);
  ck_assert_errno (riemann_attribute_set_value (NULL, "value"), EINVAL);
  ck_assert_errno (riemann_attribute_set_value (attrib, NULL), EINVAL);
  ck_assert_errno (riemann_attribute_set (NULL, "key", "value"), EINVAL);
  ck_assert_errno (riemann_attribute_set (attrib, NULL, "value"), EINVAL);
  ck_assert_errno (riemann_attribute_set (attrib, "key", NULL), EINVAL);

  ck_assert_errno (riemann_attribute_set_key (attrib, "foobar"), 0);
  ck_assert_str_eq (attrib->key, "foobar");

  ck_assert_errno (riemann_attribute_set_value (attrib, "value"), 0);
  ck_assert_str_eq (attrib->value, "value");

  ck_assert_errno (riemann_attribute_set (attrib, "key", "val"), 0);
  ck_assert_str_eq (attrib->key, "key");
  ck_assert_str_eq (attrib->value, "val");

  riemann_attribute_free (attrib);
}
END_TEST

START_TEST (test_riemann_attribute_create)
{
  riemann_attribute_t *attrib;

  ck_assert ((attrib = riemann_attribute_create (NULL, NULL)) != NULL);
  ck_assert (attrib->key == NULL);
  ck_assert (attrib->value == NULL);
  riemann_attribute_free (attrib);

  ck_assert ((attrib = riemann_attribute_create ("key", NULL)) != NULL);
  ck_assert_str_eq (attrib->key, "key");
  ck_assert (attrib->value == NULL);
  riemann_attribute_free (attrib);

  ck_assert ((attrib = riemann_attribute_create (NULL, "value")) != NULL);
  ck_assert (attrib->key == NULL);
  ck_assert_str_eq (attrib->value, "value");
  riemann_attribute_free (attrib);
}
END_TEST

START_TEST (test_riemann_attribute_clone)
{
  riemann_attribute_t *attrib, *clone;

  errno = 0;
  ck_assert (riemann_attribute_clone (NULL) == NULL);
  ck_assert_errno (-errno, EINVAL);

  attrib = riemann_attribute_create ("key", "value");
  clone = riemann_attribute_clone (attrib);

  ck_assert (clone != NULL);
  ck_assert (attrib != clone);
  ck_assert (attrib->key != clone->key);
  ck_assert (attrib->value != clone->value);

  ck_assert_str_eq (attrib->key, clone->key);
  ck_assert_str_eq (attrib->value, clone->value);

  riemann_attribute_free (attrib);
  riemann_attribute_free (clone);
}
END_TEST

static TCase *
test_riemann_attributes (void)
{
  TCase *tests;

  tests = tcase_create ("Attributes");
  tcase_add_test (tests, test_riemann_attribute_new_and_free);
  tcase_add_test (tests, test_riemann_attribute_set);
  tcase_add_test (tests, test_riemann_attribute_create);
  tcase_add_test (tests, test_riemann_attribute_clone);

  return tests;
}
