// Responsible for:
//   Adding an import statement to the current document

#ifndef JAVA_IMPORTS_H_
#define JAVA_IMPORTS_H_

#include <glib.h>

#include "text.h"

/**
 * Adds an import statement to an open document.
 * 
 * @param type_name full qualified type name to import (e.g. "java.util.List").
 * @param import_groups_order a list of type name prefix that identifies import
 *        groups in the order they are preferred to appear. An entry in import
 *        groups order may be prefixed with "static:" to indicate that only static
 *        imports with the given prefix are part of the import group.
 * @param text document text.
 */
void java_import(const gchar* type_name, const gchar** import_groups_order, 
                 Text* text);

#endif  // JAVA_IMPORTS_H_
