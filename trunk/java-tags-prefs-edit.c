#include "java-tags-prefs-edit.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-prefs.h"

extern gchar* java_tags_prefs_dlg;

static const gchar* SCOPE_GLOBAL = "Global";
static const gchar* SCOPE_PROJECT = "Project";

static void jt_prefs_dlg_init_tree(GtkTreeView* tree_view);
static void jt_prefs_dlg_set_paths(const gchar* scope, const gchar** paths, GtkListStore* store);
static gchar** jt_prefs_dlg_get_paths(GtkListStore* store, const gchar* scope);
static void jt_prefs_dlg_add(const gchar* scope, GtkListStore* paths_store);
static void jt_prefs_dlg_on_add_global(GtkButton* button, GtkListStore* paths_store);
static void jt_prefs_dlg_on_add_project(GtkButton* button, GtkListStore* paths_store);
static void jt_prefs_dlg_on_toggle_scope(GtkButton* button, GtkTreeView* paths_tree);
static void jt_prefs_dlg_on_remove(GtkButton* button, GtkTreeView* paths_tree);
static void jt_prefs_dlg_set_import_groups(const gchar** import_groups, GtkTextView* text_view);
static gchar** jt_prefs_dlg_get_import_groups(GtkTextView* text_view);

JTPrefsDlg* jt_prefs_dlg_new() {
  JTPrefsDlg* prefs_dlg = g_new(JTPrefsDlg, 1);

  // Load dialog resources.
  prefs_dlg->builder = gtk_builder_new();
  gtk_builder_add_from_string(prefs_dlg->builder, java_tags_prefs_dlg, -1, NULL);
  prefs_dlg->dlg = (GtkDialog*) gtk_builder_get_object(prefs_dlg->builder, "prefs_dlg");
  prefs_dlg->paths_tree = (GtkTreeView*) gtk_builder_get_object(prefs_dlg->builder, "paths_tree");
  prefs_dlg->paths_store = (GtkListStore*) gtk_builder_get_object(prefs_dlg->builder, "paths_store");
  prefs_dlg->import_groups = (GtkTextView*) gtk_builder_get_object(prefs_dlg->builder, "import_groups");

  // Init paths tree.
  jt_prefs_dlg_init_tree(prefs_dlg->paths_tree);

  // Init buttons.
  g_signal_connect((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "add_global"), "clicked",
                   G_CALLBACK(jt_prefs_dlg_on_add_global), prefs_dlg->paths_store);
  g_signal_connect((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "add_project"), "clicked",
                   G_CALLBACK(jt_prefs_dlg_on_add_project), prefs_dlg->paths_store);
  g_signal_connect((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "toggle_scope"), "clicked",
                   G_CALLBACK(jt_prefs_dlg_on_toggle_scope), prefs_dlg->paths_tree);
  g_signal_connect((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "remove"), "clicked",
                   G_CALLBACK(jt_prefs_dlg_on_remove), prefs_dlg->paths_tree);

  return prefs_dlg;
}

void jt_prefs_dlg_free(JTPrefsDlg* prefs_dlg) {
  gtk_widget_destroy((GtkWidget*) prefs_dlg->dlg);
  g_object_unref(prefs_dlg->builder);
}

gint jt_prefs_dlg_run(JTPrefsDlg* prefs_dlg) {
  gtk_dialog_set_default_response(prefs_dlg->dlg, GTK_RESPONSE_OK);
  return gtk_dialog_run(prefs_dlg->dlg) == GTK_RESPONSE_OK;
}

void jt_prefs_dlg_set(JTPrefsDlg* prefs_dlg,
                      JavaTagsPrefs* global_prefs, JavaTagsPrefs* project_prefs) {
  jt_prefs_dlg_set_paths(SCOPE_GLOBAL, (const gchar**) global_prefs->paths, prefs_dlg->paths_store);
  jt_prefs_dlg_set_import_groups((const gchar**) global_prefs->import_groups, prefs_dlg->import_groups);
  if (project_prefs) {
    jt_prefs_dlg_set_paths(SCOPE_PROJECT, (const gchar**) project_prefs->paths, prefs_dlg->paths_store);
  } else {
    gtk_widget_set_sensitive((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "add_project"), FALSE);
    gtk_widget_set_sensitive((GtkWidget*) gtk_builder_get_object(prefs_dlg->builder, "toggle_scope"), FALSE);
  }
}

void jt_prefs_dlg_get(JTPrefsDlg* prefs_dlg,
                      JavaTagsPrefs* global_prefs, JavaTagsPrefs* project_prefs) {
  java_tags_prefs_reset(global_prefs);
  if (project_prefs) {
    java_tags_prefs_reset(project_prefs);
  }
  global_prefs->paths = jt_prefs_dlg_get_paths(prefs_dlg->paths_store, SCOPE_GLOBAL);
  global_prefs->import_groups = jt_prefs_dlg_get_import_groups(prefs_dlg->import_groups);
  if (project_prefs) {
    project_prefs->paths = jt_prefs_dlg_get_paths(prefs_dlg->paths_store, SCOPE_PROJECT);
  }
}

static void jt_prefs_dlg_init_tree(GtkTreeView* tree_view) {
  gtk_tree_view_insert_column_with_attributes(tree_view, -1,
      "Path", gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_insert_column_with_attributes(tree_view, -1,
      "Scope", gtk_cell_renderer_text_new(), "text", 1, NULL);
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(tree_view), GTK_SELECTION_BROWSE);
}

static void jt_prefs_dlg_set_paths(const gchar* scope, const gchar** paths, GtkListStore* store) {
  if (paths) {
    const gchar** path;
    for (path = paths; *path; ++path) {
      GtkTreeIter store_iter;
      gtk_list_store_append(store, &store_iter);
      gtk_list_store_set(store, &store_iter,
                         0, *path,
                         1, scope,
                         -1);

    }
  }
}

static gchar** jt_prefs_dlg_get_paths(GtkListStore* store, const gchar* scope) {
  GPtrArray* paths_array = g_ptr_array_new();
  GtkTreeModel* model = (GtkTreeModel*) store;
  gboolean has_iter;
  GtkTreeIter iter;
  for (has_iter = gtk_tree_model_get_iter_first(model, &iter); has_iter;
       has_iter = gtk_tree_model_iter_next(model, &iter)) {
    gchar* path_scope;
    gtk_tree_model_get(model, &iter, 1, &path_scope, -1);
    if (0 == g_strcmp0(path_scope, scope)) {
      gchar* path;
      gtk_tree_model_get(model, &iter, 0, &path, -1);
      g_ptr_array_add(paths_array, path);
    }
    g_free(path_scope);
  }
  gchar** paths = g_new0(gchar*, paths_array->len+1);
  int i;
  for (i = 0; i < paths_array->len; ++i) {
    paths[i] = (gchar*) paths_array->pdata[i];
  }
  g_ptr_array_unref(paths_array);
  return paths;
}

static void jt_prefs_dlg_add(const gchar* scope, GtkListStore* store) {
  GtkDialog* dlg = (GtkDialog*) gtk_file_chooser_dialog_new(
      "Select path", /*parent*/NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
      GTK_STOCK_OK, GTK_RESPONSE_OK,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);
  if (gtk_dialog_run(dlg) == GTK_RESPONSE_OK) {
    gchar* path = gtk_file_chooser_get_filename((GtkFileChooser*) dlg);
    if (path) {
      GtkTreeIter store_iter;
      gtk_list_store_append(store, &store_iter);
      gtk_list_store_set(store, &store_iter,
                         0, path,
                         1, scope,
                         -1);
    }
    g_free(path);
  }
  gtk_widget_destroy((GtkWidget*) dlg);
}

static void jt_prefs_dlg_on_add_global(GtkButton* button, GtkListStore* paths_store) {
  jt_prefs_dlg_add(SCOPE_GLOBAL, paths_store);
}

static void jt_prefs_dlg_on_add_project(GtkButton* button, GtkListStore* paths_store) {
  jt_prefs_dlg_add(SCOPE_PROJECT, paths_store);
}

static void jt_prefs_dlg_on_toggle_scope(GtkButton* button, GtkTreeView* paths_tree) {
}

static void jt_prefs_dlg_on_remove(GtkButton* button, GtkTreeView* paths_tree) {
  GtkTreeSelection* selection = gtk_tree_view_get_selection(paths_tree);
  GtkTreeIter iter;
  if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
    GtkTreeModel* model = gtk_tree_view_get_model(paths_tree);
    // Select next
    GtkTreeIter* new_iter = gtk_tree_iter_copy(&iter);
    if (gtk_tree_model_iter_next(model, new_iter)) {
      gtk_tree_selection_select_iter(selection, new_iter);
    }
    gtk_tree_iter_free(new_iter);
    // Remove
    gtk_list_store_remove((GtkListStore*) model, &iter);
  }
}

static void jt_prefs_dlg_set_import_groups(const gchar** import_groups, GtkTextView* text_view) {
  gchar* text = import_groups ? g_strjoinv("\n", (gchar**) import_groups) : g_strdup("");
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_view);
  gtk_text_buffer_set_text (buffer, text, -1);
  g_free(text);
}

static gchar** jt_prefs_dlg_get_import_groups(GtkTextView* text_view) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_view);
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gchar* text = gtk_text_iter_get_text(&start, &end);
  gchar** import_groups = g_strsplit(text, "\n", -1);
  g_free(text);
  return import_groups;
}
