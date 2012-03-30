#ifndef TEST_H_
#define TEST_H_

#include <glib.h>

void test_setup(const char* test);
int test_set_error(const char* error);
void test_teardown();

#define RUN_TEST(test) \
  { \
    test_setup(#test); \
    test(); \
    test_teardown(); \
  }


#define ASSERT_EQ(x, y) \
  if ((x) != (y)) { \
    g_warning("ASSERT(%s = %s). Expected %d. Actual %d.", #x, #y, (x), (y)); \
    test_set_error("assert failed"); \
  }

#define ASSERT_STR_EQ(x, y) \
  if (strcmp(x, y)) { \
    g_warning("ASSERT(%s == %s): Expected %s. Actual %s.", #x, #y, (x), (y)); \
    test_set_error("assert failed"); \
  }

#define ASSERT_NOT_NULL(x) \
  if (!x) { \
    g_warning("ASSERT(%s): Expected not null.", #x); \
    test_set_error("assert failed"); \
  }

#endif // TEST_H_
