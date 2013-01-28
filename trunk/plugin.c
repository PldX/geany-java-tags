// Responsible for:
//  Plugin initialization and cleanup
//  Menu initialization, handling and cleanup
//  Keybindings

#include "java-tags-select.h"
#include "java-imports.h"
#include "java-tags-prefs.h"
#include "java-tags-prefs-edit.h"
#include "java-tags-store.h"
#include "java-tags-parse.h"
#include "sci-utils.h"
#include "sci-text.h"

#include "geanyplugin.h"

GeanyPlugin* geany_plugin;
GeanyData* geany_data;
GeanyFunctions* geany_functions;

PLUGIN_VERSION_CHECK(147)
PLUGIN_SET_INFO("Geany Java Tags", "Tags browse and import for java files/projects.",
                "0.0", "Razvan Constantin <razvanc@google.com>");

////////////////////////////////////////////////////////////////////////////////
// Global plugin data.
////////////////////////////////////////////////////////////////////////////////

typedef struct _JavaTagsPlugin {
  // Prefs.
  JavaTagsPrefs* global_prefs;
  JavaTagsPrefs* project_prefs;
  // Store.
  JavaTagsStore* tags_store;
  // Parser and its mutex.
  JavaTagsParser* tags_parser;
  GMutex* tags_parser_mutex;
  // Menus.
  GtkWidget* import_menu_item;
} JavaTagsPlugin;
static JavaTagsPlugin jtp_instance;

static void jtp_init(JavaTagsPlugin* jtp) {
  jtp->global_prefs = NULL;
  jtp->project_prefs = NULL;
  jtp->tags_store = NULL;
  jtp->tags_parser = NULL;
  jtp->tags_parser_mutex = g_mutex_new();
  jtp->import_menu_item = NULL;
}

static void jtp_reload(JavaTagsPlugin* jtp);

static void jtp_load_prefs(JavaTagsPlugin* jtp);
static void jtp_save_prefs(JavaTagsPlugin* jtp);
static void jtp_unload_prefs(JavaTagsPlugin* jtp);
static void jtp_on_configure_response(GtkDialog* dlg, gint resp, JTPrefsDlg* prefs_dlg);

static void jtp_load_tags(JavaTagsPlugin* jtp);
static void jtp_unload_tags(JavaTagsPlugin* jtp);
static void jtp_reload_tags(JavaTagsPlugin* jtp);

static void jtp_setup_menus(JavaTagsPlugin* jtp);
static void jtp_cleanup_menus(JavaTagsPlugin* jtp);
static void jtp_on_activate_import(GtkMenuItem* menuitem, JavaTagsPlugin* jtp);

static void jtp_setup_key_bindings(JavaTagsPlugin* jtp);
static void jtp_cleanup_key_bindings(JavaTagsPlugin* jtp);

void jtp_reload(JavaTagsPlugin* jtp) {
  jtp_unload_tags(jtp);
  jtp_unload_prefs(jtp);
  jtp_load_prefs(jtp);
  jtp_load_tags(jtp);
}

////////////////////////////////////////////////////////////////////////////////
// Preferences
////////////////////////////////////////////////////////////////////////////////

static void jtp_load_prefs(JavaTagsPlugin* jtp) {
  jtp->global_prefs = java_tags_prefs_new_global(NULL);
  java_tags_prefs_load(jtp->global_prefs);
  if (geany_data->app->project && geany_data->app->project->file_name &&
      g_file_test(geany_data->app->project->file_name, G_FILE_TEST_IS_REGULAR)) {
    jtp->project_prefs = java_tags_prefs_new_project(geany_data->app->project->file_name);
    java_tags_prefs_load(jtp->project_prefs);
  } else {
    jtp->project_prefs = NULL;
  }
}

static void jtp_save_prefs(JavaTagsPlugin* jtp) {
  java_tags_prefs_save(jtp->global_prefs);
  if (jtp->project_prefs) {
    java_tags_prefs_save(jtp->project_prefs);
  }
}

static void jtp_unload_prefs(JavaTagsPlugin* jtp) {
  java_tags_prefs_free(jtp->global_prefs);
  jtp->global_prefs = NULL;
  java_tags_prefs_free(jtp->project_prefs);
  jtp->project_prefs = NULL;
}

static void jtp_on_configure_response(GtkDialog* dlg, gint resp, JTPrefsDlg* prefs_dlg) {
  if (resp == GTK_RESPONSE_OK || resp == GTK_RESPONSE_APPLY) {
    JavaTagsPlugin* jtp = &jtp_instance;
    jt_prefs_dlg_get(prefs_dlg, jtp->global_prefs, jtp->project_prefs);
    jtp_save_prefs(jtp);
    jtp_reload_tags(jtp);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Tags store.
////////////////////////////////////////////////////////////////////////////////

static void jtp_show_parser_stats(JavaTagsParser* tags_parser) {
  JavaTagsParserStats stats;
  jt_parser_get_stats(tags_parser, &stats);
  ui_set_statusbar(FALSE, "Java tags: %d", stats.tags);
}

static gboolean jtp_refresh_parser_stats(JavaTagsParser* tags_parser) {
  // Show stats if active.
  if (jtp_instance.tags_parser == tags_parser) {
    jtp_show_parser_stats(tags_parser);
  }
  // Release parser and uninstall when no longer working.
  // NOTE: It might be too early if worker thread is not started yet... to be solved in parser.
  if (!jt_parser_working(tags_parser)) {
    jt_parser_release(tags_parser);
    return FALSE; // Stops further notifications.
  }
  return TRUE;
}

static void jtp_load_tags(JavaTagsPlugin* jtp) {
  const gchar* project_base_path = "";
  if (geany_data->app->project && geany_data->app->project->base_path) {
    project_base_path = geany_data->app->project->base_path;
  }
  GPtrArray* paths = g_ptr_array_new_with_free_func((GDestroyNotify) g_free);
  gchar** ppath;
  if (jtp->global_prefs && jtp->global_prefs->paths) {
    for (ppath = jtp->global_prefs->paths; *ppath; ++ppath) {
      g_ptr_array_add(paths, (gpointer) g_str_replace(*ppath, "%p", project_base_path));
    }
  }
  if (jtp->project_prefs && jtp->project_prefs->paths) {
    for (ppath = jtp->project_prefs->paths; *ppath; ++ppath) {
      g_ptr_array_add(paths, (gpointer) g_str_replace(*ppath, "%p", project_base_path));
    }
  }
  jtp->tags_store = java_tags_store_new();
  jtp->tags_parser = jt_parser_new(paths, jtp->tags_store);
  jt_parser_add_ref(jtp->tags_parser); // Reference for jtp_refresh_parser_stats.
  plugin_timeout_add(geany_plugin, 3, (GSourceFunc) jtp_refresh_parser_stats, jtp->tags_parser);
  jt_parser_start(jtp->tags_parser);
}

static void jtp_unload_tags(JavaTagsPlugin* jtp) {
  if (jtp->tags_parser) {
    jt_parser_abort(jtp->tags_parser);
    jt_parser_orphan(jtp->tags_parser); // Release the store.
    jtp->tags_parser = NULL;
  } else {
    java_tags_store_free(jtp->tags_store);
  }
  jtp->tags_store = NULL;
}

static void jtp_reload_tags(JavaTagsPlugin* jtp) {
  jtp_unload_tags(jtp);
  jtp_load_tags(jtp);
}

////////////////////////////////////////////////////////////////////////////////
// Menus
////////////////////////////////////////////////////////////////////////////////

static void jtp_setup_menus(JavaTagsPlugin* jtp) {
  // Tools
  GtkWidget* tools_menu = geany_data->main_widgets->tools_menu;
  // Tools -> Java Import
  jtp->import_menu_item = gtk_menu_item_new_with_mnemonic("Java _Import");
  gtk_widget_show(jtp->import_menu_item);
  gtk_container_add((GtkContainer*) tools_menu, jtp->import_menu_item);
  plugin_signal_connect(geany_plugin, (GObject*)jtp->import_menu_item, "activate",
      FALSE, G_CALLBACK(jtp_on_activate_import), jtp);
}

static void jtp_cleanup_menus(JavaTagsPlugin* jtp) {
  gtk_widget_destroy(jtp->import_menu_item);
}

static void jtp_on_activate_import(GtkMenuItem* menuitem, JavaTagsPlugin* jtp) {
  GeanyDocument* doc = document_get_current();
  if (doc) {
    gboolean resume_parser = FALSE;
    if (jtp->tags_parser) {
      jtp_show_parser_stats(jtp->tags_parser); // Ensure up-to-date stats.
      if (jt_parser_working(jtp->tags_parser)) {
        jt_parser_suspend(jtp->tags_parser);
        resume_parser = TRUE;
      } else {
        JavaTagsParser* parser = jtp->tags_parser;
        jtp->tags_parser = NULL;
        jt_parser_release(parser);
      }
    }

    ScintillaObject* sci = doc->editor->sci;
    gchar* init_tag = sci_get_current_word_text(sci);

    gchar* tag = java_tags_select(jtp->tags_store, init_tag);
    if (tag) {
      msgwin_status_add("Selected %s", tag);
      Text* sci_text = sci_text_new(sci);
      java_import(tag, (const gchar**) jtp->global_prefs->import_groups, sci_text);
      text_free(sci_text);
    }

    g_free(tag);
    g_free(init_tag);
    
    if (resume_parser) {
      jt_parser_resume(jtp->tags_parser);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Key bindings
////////////////////////////////////////////////////////////////////////////////

static void jtp_setup_key_bindings(JavaTagsPlugin* jtp) {
  GeanyKeyGroup* keyGroup;
  keyGroup = plugin_set_key_group(geany_plugin, "Java Tags", /*items*/ 1, NULL);
  keybindings_set_item(keyGroup, /*key_id*/ 0, /*callback*/ NULL,
                       /*key*/ 0, (GdkModifierType) 0, (gchar*) "import",
                       (gchar*) "Import", jtp->import_menu_item);
}

static void jtp_cleanup_key_bindings(JavaTagsPlugin* jtp) {
}


////////////////////////////////////////////////////////////////////////////////
// Entry point.
////////////////////////////////////////////////////////////////////////////////

void plugin_init(GeanyData* data) {
  // Init plugin data.
  jtp_init(&jtp_instance);
  // Load preferences.
  jtp_load_prefs(&jtp_instance);
  // Load store.
  jtp_load_tags(&jtp_instance);
  // Setup menus.
  jtp_setup_menus(&jtp_instance);
  // Setup key bindings.
  jtp_setup_key_bindings(&jtp_instance);
}

GtkWidget* plugin_configure(GtkDialog* dlg) {
  JTPrefsDlg* prefs_dlg = jt_prefs_dlg_new();
  // Create dialog.
  JavaTagsPlugin* jtp = &jtp_instance;
  jt_prefs_dlg_set(prefs_dlg, jtp->global_prefs, jtp->project_prefs);
  // Connect response event on master dialog.
  g_signal_connect(dlg, "response", G_CALLBACK(jtp_on_configure_response), prefs_dlg);
  // Extract page content.
  GtkWidget* jt_prefs_widget = (GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "prefs_vbox");
  g_object_ref(jt_prefs_widget);
  gtk_container_remove((GtkContainer*)gtk_widget_get_parent(jt_prefs_widget), jt_prefs_widget);
  gtk_widget_show_all(jt_prefs_widget);
  // Setup auto deletion.
  g_object_set_data_full((GObject*) jt_prefs_widget, "java.tags.prefs.dlg",
                         prefs_dlg, (GDestroyNotify) jt_prefs_dlg_free);
  return jt_prefs_widget;
}

static void plugin_project_new(GObject* unused1, GtkWidget *notebook, gpointer unused2) {
  jtp_reload(&jtp_instance);
}
static void plugin_project_open(GObject* unused1, gpointer unused2) {
  jtp_reload(&jtp_instance);
}
static void plugin_project_close(GObject* unused1, gpointer unused2) {
  jtp_reload(&jtp_instance);
}
PluginCallback plugin_callbacks[] = {
  { "project-dialog-confirmed", (GCallback) &plugin_project_new, TRUE, NULL },
  { "project-open", (GCallback) &plugin_project_open, TRUE, NULL },
  { "project-close", (GCallback) &plugin_project_close, FALSE, NULL },
  { NULL, NULL, FALSE, NULL }
};

void plugin_cleanup(void) {
  // Cleanup key bindings.
  jtp_cleanup_key_bindings(&jtp_instance);
  // Cleanup menus.
  jtp_cleanup_menus(&jtp_instance);
  // Cleanup tags.
  jtp_unload_tags(&jtp_instance);
  // Cleanup preferences.
  jtp_unload_prefs(&jtp_instance);
}
