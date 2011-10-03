#ifndef JAVA_TAGS_PREFS_EDIT_H_
#define JAVA_TAGS_PREFS_EDIT_H_

#include <glib.h>
#include <gtk/gtk.h>

#include "java-tags-prefs.h"

typedef struct _JTPrefsDlg {
  GtkBuilder* builder;
  GtkDialog* dlg;
  GtkTreeView* paths_tree;
  GtkListStore* paths_store;
  GtkTextView* import_groups;
} JTPrefsDlg;

// Creates a new initialized dialog for editing java tags plugin preferences.
// @return a new preferences dialog. Caller takes ownership (use jt_prefs_dlg_free to release).
JTPrefsDlg* jt_prefs_dlg_new();

// Destroys any data associated with the preferences dialog (except actual preferences).
void jt_prefs_dlg_free(JTPrefsDlg* dlg);

// Updates the data in the dialog from the content of the preferences.
void jt_prefs_dlg_set(JTPrefsDlg* dlg, JavaTagsPrefs* global_prefs, JavaTagsPrefs* project_prefs);
// Updates the data in the preferences with the content of the dialog.
void jt_prefs_dlg_get(JTPrefsDlg* dlg, JavaTagsPrefs* global_prefs, JavaTagsPrefs* project_prefs);

// Runs the dialog and updates the preferences if needed.
// @return true if ok/accept was selected.
gboolean jt_prefs_dlg_run(JTPrefsDlg* dlg);

#endif // JAVA_TAGS_PREFS_EDIT_H_
