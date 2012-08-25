#include "java-tags-prefs-edit.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-prefs.h"

extern gchar* java_tags_prefs_dlg;

static void jt_prefs_dlg_set_lines(const gchar** import_groups, GtkTextView* text_view);
static gchar** jt_prefs_dlg_get_lines(GtkTextView* text_view);

JTPrefsDlg* jt_prefs_dlg_new() {
  JTPrefsDlg* prefs_dlg = g_new(JTPrefsDlg, 1);

  // Load dialog resources.
  prefs_dlg->builder = gtk_builder_new();
  gtk_builder_add_from_string(prefs_dlg->builder, java_tags_prefs_dlg, -1, NULL);
  prefs_dlg->dlg = (GtkDialog*) gtk_builder_get_object(prefs_dlg->builder, "prefs_dlg");
  prefs_dlg->global_paths = (GtkTextView*) gtk_builder_get_object(prefs_dlg->builder, "global_paths");
  prefs_dlg->project_paths = (GtkTextView*) gtk_builder_get_object(prefs_dlg->builder, "project_paths");
  prefs_dlg->import_groups = (GtkTextView*) gtk_builder_get_object(prefs_dlg->builder, "import_groups");

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
  jt_prefs_dlg_set_lines((const gchar**) global_prefs->paths, prefs_dlg->global_paths);
  jt_prefs_dlg_set_lines((const gchar**) global_prefs->import_groups, prefs_dlg->import_groups);
  if (project_prefs) {
    jt_prefs_dlg_set_lines((const gchar**) project_prefs->paths, prefs_dlg->project_paths);
  } else {
    gtk_widget_set_sensitive((GtkWidget*) prefs_dlg->project_paths, FALSE);
  }
}

void jt_prefs_dlg_get(JTPrefsDlg* prefs_dlg,
                      JavaTagsPrefs* global_prefs, JavaTagsPrefs* project_prefs) {
  java_tags_prefs_reset(global_prefs);
  if (project_prefs) {
    java_tags_prefs_reset(project_prefs);
  }
  global_prefs->paths = jt_prefs_dlg_get_lines(prefs_dlg->global_paths);
  global_prefs->import_groups = jt_prefs_dlg_get_lines(prefs_dlg->import_groups);
  if (project_prefs) {
    project_prefs->paths = jt_prefs_dlg_get_lines(prefs_dlg->project_paths);
  }
}

static void jt_prefs_dlg_set_lines(const gchar** lines, GtkTextView* text_view) {
  gchar* text = lines ? g_strjoinv("\n", (gchar**) lines) : g_strdup("");
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_view);
  gtk_text_buffer_set_text (buffer, text, -1);
  g_free(text);
}

static gchar** jt_prefs_dlg_get_lines(GtkTextView* text_view) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_view);
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gchar* text = gtk_text_iter_get_text(&start, &end);
  gchar** lines = g_strsplit(text, "\n", -1);
  g_free(text);
  return lines;
}
