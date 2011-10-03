// Tags selection dialog.

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-prefs.h"
#include "java-tags-prefs-edit.h"

int main(int argc, char** argv) {
  gtk_init (&argc, &argv);
  JavaTagsPrefs* global_prefs = java_tags_prefs_new_global(NULL);
  JavaTagsPrefs* project_prefs = java_tags_prefs_new_project(
      "/Users/razvanc/projects/geany-ctags.geany");
  
  java_tags_prefs_load(global_prefs);
  java_tags_prefs_load(project_prefs);
  
  JTPrefsDlg* prefs_dlg = jt_prefs_dlg_new();
  jt_prefs_dlg_set(prefs_dlg, global_prefs, project_prefs);
  if (jt_prefs_dlg_run(prefs_dlg)) {
    jt_prefs_dlg_get(prefs_dlg, global_prefs, project_prefs);
  }
  
  java_tags_prefs_save(global_prefs);
  java_tags_prefs_save(project_prefs);
  
  java_tags_prefs_free(project_prefs);
  java_tags_prefs_free(global_prefs);
  return 0;
}
