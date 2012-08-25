#include "utils.h"

#include <stdio.h>

#include "test.h"

void printarr(gchar** a) {
  printf("[%d]", array_len(a));
  gint i;
  for (i = 0; i < array_len(a); ++i) {
    printf(" %s", a[i]);
  }
  printf("\n");
}

void test_replace() {
  ASSERT_STR_EQ("base", g_str_replace("%p", "%p", "base"));
  ASSERT_STR_EQ("basex", g_str_replace("%px", "%p", "base"));
  ASSERT_STR_EQ("xbase", g_str_replace("x%p", "%p", "base"));
  ASSERT_STR_EQ("xbasex", g_str_replace("x%px", "%p", "base"));
  ASSERT_STR_EQ("basebase", g_str_replace("%p%p", "%p", "base"));
  ASSERT_STR_EQ("", g_str_replace("", "%p", "base"));
}

int main(int argc, char** argv) {
  RUN_TEST(test_replace);
  
  gchar** a = g_new0(gchar*, 1);
  printarr(a);
  array_add(a, "1");
  printarr(a);
  array_add(a, "2");
  printarr(a);
  array_add(a, "4");
  printarr(a);
  array_insert(a, 2, "3");
  printarr(a);
  
  return 0;
}
