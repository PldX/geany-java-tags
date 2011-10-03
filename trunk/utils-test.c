
#include "utils.h"

void printarr(gchar** a) {
  printf("[%d]", array_len(a));
  gint i;
  for (i = 0; i < array_len(a); ++i) {
    printf(" %s", a[i]);
  }
  printf("\n");
}

int main(int argc, char** argv) {
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
