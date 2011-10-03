#include "java-tags-prefs.h"

#include <glib.h>
#include <sys/stat.h>

int g_default_mode() {
  int mode = S_IRWXU;
#ifdef S_IRGRP
  mode |= S_IRGRP;
#endif
#ifdef S_IXGRP
  mode |= S_IXGRP;
#endif
#ifdef S_IXOTH
  mode |= S_IXOTH;
#endif
  return mode;
}

extern gchar* java_tags_prefs_dlg;

static JavaTagsPrefs* java_tags_prefs_new(JavaTagsPrefsType type, const gchar* prefs_file);

JavaTagsPrefs* java_tags_prefs_new_global(const gchar* config_dir) {
  gchar* prefs_file = g_build_filename(
      config_dir ? config_dir : g_get_user_config_dir(), "java-tags.conf", NULL);
  JavaTagsPrefs* prefs = java_tags_prefs_new(JAVA_TAGS_PREFS_GLOBAL, prefs_file);
  g_free(prefs_file);
  return prefs;
}

JavaTagsPrefs* java_tags_prefs_new_project(const gchar* project_file) {
  return java_tags_prefs_new(JAVA_TAGS_PREFS_PROJECT, project_file);
}

static JavaTagsPrefs* java_tags_prefs_new(JavaTagsPrefsType type, const gchar* prefs_file) {
  JavaTagsPrefs* prefs = g_new(JavaTagsPrefs, 1);
  prefs->type = type;
  prefs->prefs_file = g_strdup(prefs_file);
  prefs->paths = NULL;
  prefs->import_groups = NULL;
}

void java_tags_prefs_free(JavaTagsPrefs* prefs) {
  if (prefs) {
    java_tags_prefs_reset(prefs);
    g_free(prefs->prefs_file);
    g_free(prefs);
  }
}

void java_tags_prefs_reset(JavaTagsPrefs* prefs) {
  if (prefs) {
    g_strfreev(prefs->paths);
    prefs->paths = NULL;
    g_strfreev(prefs->import_groups);
    prefs->import_groups = NULL;
  }
}

gboolean java_tags_prefs_load(JavaTagsPrefs* prefs) {
  // Reset prefs.
  java_tags_prefs_reset(prefs);
  
  // Exit fast if file not found.
  if (!g_file_test(prefs->prefs_file, G_FILE_TEST_EXISTS)) {
    return FALSE;
  }
  
  // Load file.
  GKeyFile* key_file = g_key_file_new();
  if (!g_key_file_load_from_file(key_file, prefs->prefs_file, G_KEY_FILE_NONE, NULL)) {
    g_key_file_free(key_file);
    return FALSE;
  }
  
  // Paths.
  prefs->paths = g_key_file_get_string_list(key_file, "java_tags", "paths", NULL, NULL);
  // Import groups.
  if (prefs->type == JAVA_TAGS_PREFS_GLOBAL) {
    prefs->import_groups = g_new0(gchar*, 10);
    int i, n = -1;
    for (i = 1; i < 9; ++i) {
      gchar* key = g_strdup_printf("import_group%d", i);
      gchar* import_group = g_key_file_get_string(key_file, "import_groups", key, NULL);
      if (import_group) {
        prefs->import_groups[++n] = import_group;
      }
      g_free(key);
    }
  }
  
  // Free resources.
  g_key_file_free(key_file);
  return TRUE;
}

gboolean java_tags_prefs_save(JavaTagsPrefs* prefs) {
  // Load previous version.
  GKeyFile* key_file = g_key_file_new();
  if (g_file_test(prefs->prefs_file, G_FILE_TEST_EXISTS)) {
    if (!g_key_file_load_from_file(key_file, prefs->prefs_file, G_KEY_FILE_NONE, NULL)) {
      g_key_file_free(key_file);
      return FALSE;
    }
  }
  
  // Paths.
  g_key_file_set_string_list(key_file, "java_tags", "paths", 
                             (const gchar* const*) prefs->paths,
                             g_strv_length(prefs->paths));
  // Import groups.
  if (prefs->type == JAVA_TAGS_PREFS_GLOBAL) {
    int i, n = g_strv_length(prefs->import_groups);
    for (i = 1; i < 9; ++i) {
      gchar* key = g_strdup_printf("import_group%d", i);
      if (i-1 < n) {
        g_key_file_set_string(key_file, "import_groups", key, prefs->import_groups[i-1]);
      } else {
        g_key_file_remove_key(key_file, "import_groups", key, NULL);
      }
      g_free(key);
    }
  }
  
  // Serialize config.
  gchar* key_file_data = g_key_file_to_data(key_file, NULL, NULL);
  g_key_file_free(key_file);
  
  // Ensure directory.
  gchar* dir = g_path_get_dirname(prefs->prefs_file);
  if (g_mkdir_with_parents(dir, g_default_mode()) != 0) {
    g_free(dir);
    g_free(key_file_data);
    return FALSE;
  }
  g_free(dir);
  
  // Write file.
  if (!g_file_set_contents(prefs->prefs_file, key_file_data, -1, NULL)) {
    g_free(key_file_data);
    return FALSE;
  }
  
  // Free resources.
  g_free(key_file_data);
  return TRUE;
}
