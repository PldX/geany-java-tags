#include "java-parser.h"

#include <glib.h>
#include <string.h>

#include "java-parser-impl.h"
#include "text.h"
#include "utils.h"

// Java doc parsing logic.
static gboolean java_parse_doc(JavaParser* parser, JavaDocStructure** pdoc);
// Destroys the content of a java doc without deleting the pointer.
static void java_doc_destroy(JavaDocStructure* java_doc);

void java_parse_opts_init(JavaParseOptions* opts) {
  opts->extract_package = FALSE;
  opts->extract_imports = FALSE;
  opts->extract_types = FALSE;
  opts->extract_members = FALSE;
  opts->extract_body = FALSE;
}

JavaDocStructure* java_parse(Text* text, JavaParseOptions* opts) {
  JavaParser parser;
  parser.text = text;
  parser.opts = opts;
  parser.pos = 0;
  JavaDocStructure* doc_struct = NULL;
  java_parse_doc(&parser, &doc_struct);
  return doc_struct;
}

void java_doc_free(JavaDocStructure* doc) {
  java_doc_destroy(doc);
  g_free(doc);
}

static gboolean java_parse_doc(JavaParser* parser, JavaDocStructure** pdoc) {
  // Initialize struct.
  JavaDocStructure doc;
  doc.package_declaration = NULL;
  doc.import_declarations = g_new(JavaImportDeclaration*, 1);
  doc.import_declarations[0] = NULL;
  doc.type_declarations = g_new(JavaTypeDeclaration*, 1);
  doc.type_declarations[0] = NULL;
  
  // Package.
  JavaPackageDeclaration* package_declaration = NULL;
  if (java_parse_package(parser, parser->opts->extract_package ? &package_declaration : NULL)) {
    doc.package_declaration = package_declaration;
  }
  // Imports.
  JavaImportDeclaration* import_declaration = NULL;
  while (java_parse_import(parser, parser->opts->extract_imports ? &import_declaration : NULL)) {
    if (import_declaration) {
      doc.import_declarations = (JavaImportDeclaration**) ARRAY_ADD(
          doc.import_declarations,
          import_declaration);
    }
    import_declaration = NULL;
  }
  // Types.
  if (parser->opts->extract_types) {
    JavaTypeDeclaration* type_declaration = NULL;
    while (java_parse_type(parser, &type_declaration)) {
      array_add((gpointer*) &(doc.type_declarations), (gpointer) type_declaration);
      type_declaration = NULL;
    }
  }
  
  // Fill return value.
  if (pdoc) {
    *pdoc = STRUCT_DUP(JavaDocStructure, &doc);
  } else {
    java_doc_destroy(&doc);
  }
  return TRUE;
}

static void java_doc_destroy(JavaDocStructure* doc) {
  java_package_declaration_free(doc->package_declaration);
  ARRAY_FREE(doc->import_declarations, java_import_declaration_free);
  ARRAY_FREE(doc->type_declarations, java_type_declaration_free);
}

