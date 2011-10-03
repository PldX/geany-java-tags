// Responsible for:
//   Adding an import statement to the current document

#ifndef JAVA_IMPORTS_H_
#define JAVA_IMPORTS_H_

#include <glib.h>

#include "geanyplugin.h"  // ScintillaObject

/**
 * Adds an import statement to an open document.
 * 
 * @param import full import statement line without newline
 *        (e.g. "import java.util.List;").
 * @param import_groups_order a list of regular expressions which indicates
 *        preferred order for import groups.
 * @param sci document object.
 */
void java_import(const gchar* import, const gchar** import_groups_order, ScintillaObject* sci);

#endif  // JAVA_IMPORTS_H_
