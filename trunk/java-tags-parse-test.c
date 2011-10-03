
#include <glib.h>

#include "java-tags-store.h"
#include "java-tags-parse.h"

int main(int argc, char** argv) {
  g_thread_init(NULL);
  if (argc != 2) {
    g_warning("Usage %s path", argv[0]);
    return 2;
  }
  GPtrArray* paths = g_ptr_array_new();
  g_ptr_array_add(paths, argv[1]);
  JavaTagsStore* store = java_tags_store_new();
  JavaTagsParser* parser = jt_parser_new(paths, store);
  jt_parser_start(parser);
  jt_parser_wait(parser);
  jt_parser_free(parser);
  java_tags_store_free(store);
  g_ptr_array_free(paths, TRUE);
  return 0;
}
