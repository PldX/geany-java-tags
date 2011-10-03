#ifndef JAVA_TAGS_PREFS_H_
#define JAVA_TAGS_PREFS_H_

#include <glib.h>

typedef enum _JavaTagsPrefsType {
  JAVA_TAGS_PREFS_GLOBAL,
  JAVA_TAGS_PREFS_PROJECT,
} JavaTagsPrefsType;

typedef struct _JavaTagsPrefs {
  JavaTagsPrefsType type;
  gchar* prefs_file;
  
  gchar** paths;
  gchar** import_groups;
} JavaTagsPrefs;

JavaTagsPrefs* java_tags_prefs_new_global(const gchar* config_dir);
JavaTagsPrefs* java_tags_prefs_new_project(const gchar* project_file);
void java_tags_prefs_reset(JavaTagsPrefs* prefs);
void java_tags_prefs_free(JavaTagsPrefs* prefs);

gboolean java_tags_prefs_load(JavaTagsPrefs* prefs);
gboolean java_tags_prefs_save(JavaTagsPrefs* prefs);

#endif // JAVA_TAGS_PREFS_H_
