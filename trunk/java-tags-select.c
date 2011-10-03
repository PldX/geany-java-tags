// Tags selection dialog.

#include "java-tags-select.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-store.h"

// TODO: Move this to gtk utils.
static void gtk_tree_view_select_first(GtkTreeView* tree_view);

extern gchar* java_tags_select_dlg;

typedef struct _JavaTagsSelectDlg {
  JavaTagsStore* tags_store;
  GtkEntry* tag_entry;
  GtkListStore* list_store;
  GtkTreeView* tree_view;
  GtkDialog* dlg;
} JavaTagsSelectDlg;

static void java_tags_select_on_tag_changed(GtkEntry* tag_entry, JavaTagsSelectDlg* select_dlg);
static void java_tags_select_on_find_clicked(GtkButton* tag_find, JavaTagsSelectDlg* select_dlg);
static void java_tags_select_init_tree(GtkTreeView* tree);
static void java_tags_select_on_row_select(GtkTreeView* tree_view, GtkTreePath* path,
                                           GtkTreeViewColumn* column, JavaTagsSelectDlg* select_dlg);
static void java_tags_select_update_tree(JavaTagsSelectDlg* select_dlg, const gchar* tag);

gchar* java_tags_select(JavaTagsStore* tags_store, gchar* tag) {
  // Dlg data.
  JavaTagsSelectDlg select_dlg;
  select_dlg.tags_store = tags_store;

  // Load dialog resources.
  GtkBuilder* builder = gtk_builder_new();
  gtk_builder_add_from_string(builder, java_tags_select_dlg, -1, NULL);
  select_dlg.dlg = (GtkDialog*) gtk_builder_get_object(builder, "java_tags_select_dlg");
  select_dlg.tag_entry = (GtkEntry*) gtk_builder_get_object(builder, "tag_entry");
  select_dlg.list_store = (GtkListStore*) gtk_builder_get_object(builder, "tags_store");
  select_dlg.tree_view = (GtkTreeView*) gtk_builder_get_object(builder, "tags_tree");

  // Initialize edit.
  gtk_entry_set_text(select_dlg.tag_entry, tag ? tag : "");
  g_signal_connect((GtkWidget*) select_dlg.tag_entry, "changed",
                   G_CALLBACK(java_tags_select_on_tag_changed), (gpointer) &select_dlg);

  // Initialize find.
  GtkButton* tag_find = (GtkButton*) gtk_builder_get_object(builder, "tag_find");
  g_signal_connect((GtkWidget*) tag_find, "clicked",
                   G_CALLBACK(java_tags_select_on_find_clicked), (gpointer) &select_dlg);

  // Initialize tree.
  java_tags_select_init_tree(select_dlg.tree_view);
  g_signal_connect((GtkWidget*) select_dlg.tree_view, "row-activated",
                   G_CALLBACK(java_tags_select_on_row_select), (gpointer) &select_dlg);

  // Set data.
  java_tags_select_update_tree(&select_dlg, tag ? tag : "");
  gtk_tree_view_select_first(select_dlg.tree_view);

  // Run dialog.
  gtk_dialog_set_default_response(select_dlg.dlg, GTK_RESPONSE_OK);
  gint result = gtk_dialog_run(select_dlg.dlg);

  // Return result.
  gchar* selected_tag_fullname = NULL;
  if (result == GTK_RESPONSE_OK) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(select_dlg.tree_view);
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
      gtk_tree_model_get((GtkTreeModel*) select_dlg.list_store, &iter, 1, &selected_tag_fullname, -1);
    }
  }

  // Free resources.
  gtk_widget_destroy((GtkWidget*) select_dlg.dlg);
  g_object_unref(builder);

  return selected_tag_fullname;
}

static void java_tags_select_on_tag_changed(GtkEntry* tag_entry, JavaTagsSelectDlg* select_dlg) {
  const gchar* tag = gtk_entry_get_text(tag_entry);
  java_tags_select_update_tree(select_dlg, tag);
  gtk_tree_view_select_first(select_dlg->tree_view);
}

static void java_tags_select_on_find_clicked(GtkButton* tag_find, JavaTagsSelectDlg* select_dlg) {
  java_tags_select_on_tag_changed(select_dlg->tag_entry, select_dlg);
}

static void java_tags_select_init_tree(GtkTreeView* tree_view) {
  gtk_tree_view_insert_column_with_attributes(tree_view, -1,
      "Tag", gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_insert_column_with_attributes(tree_view, -1,
      "Details", gtk_cell_renderer_text_new(), "text", 1, NULL);
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(tree_view), GTK_SELECTION_BROWSE);
}

static void gtk_tree_view_select_first(GtkTreeView* tree_view) {
  GtkTreeModel* tree_model = gtk_tree_view_get_model(tree_view);
  GtkTreeIter first;
  if (gtk_tree_model_get_iter_first(tree_model, &first)) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree_view);
    gtk_tree_selection_select_iter(selection, &first);
  }
}

static void java_tags_select_on_row_select(GtkTreeView* tree_view,
                                           GtkTreePath* path, GtkTreeViewColumn* column,
                                           JavaTagsSelectDlg* select_dlg) {
  gtk_dialog_response(select_dlg->dlg, GTK_RESPONSE_OK);
}

static void java_tags_select_update_tree(JavaTagsSelectDlg* select_dlg, const gchar* tag) {
  gtk_list_store_clear(select_dlg->list_store);
  JavaTagsStoreIter* tags_store_iter = java_tags_store_search(select_dlg->tags_store, (gpointer) tag);
  for (; !java_tags_store_iter_done(tags_store_iter); java_tags_store_iter_next(tags_store_iter)) {
    JavaTag* java_tag = java_tags_store_iter_get(tags_store_iter);
    if (!g_str_has_prefix(java_tag->name, tag)) {
      break;
    }
    GtkTreeIter list_store_iter;
    gtk_list_store_append(select_dlg->list_store, &list_store_iter);
    gtk_list_store_set(select_dlg->list_store, &list_store_iter,
                       0, java_tag->name,
                       1, java_tag->fullname,
                       -1);
  }
  java_tags_store_iter_free(tags_store_iter);
}
