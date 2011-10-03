// Tags selection dialog.

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-parse.h"
#include "java-tags-select.h"
#include "java-tags-store.h"

int main(int argc, char** argv) {
  gtk_init (&argc, &argv);
  JavaTagsStore* store = java_tags_store_new();
  g_thread_join(java_tags_parse_async("/usr/share/java/junit-4.6", store));
  g_thread_join(java_tags_parse_async("/usr/share/java", store));

  gchar* selection = java_tags_select(store, "Test");
  g_message("Selected: %s", selection);
  g_free(selection);

  java_tags_store_free(store);
  return 0;
}
