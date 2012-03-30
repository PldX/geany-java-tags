#include "test.h"

#include <glib.h>

static int has_error = 0;

void test_setup(const char* test) {
  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "TEST: %s", test);
  has_error = 0;
}

int test_set_error(const char* error) {
  has_error = 1;
  return 0;
}

void test_teardown() {
  if (has_error) {
    g_warning("FAILURE");
  } else {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "SUCCESS");
  }
  has_error = 0;
}
