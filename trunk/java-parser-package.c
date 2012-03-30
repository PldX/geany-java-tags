#include "java-parser.h"
#include "java-parser-impl.h"

#include <glib.h>
#include <string.h>

#include "utils.h"

// Destroys the content of a package declaration without deleting the pointer.
static void java_package_declaration_destroy(JavaPackageDeclaration* package_declaration);

gboolean java_parse_package(JavaParser* parser, JavaPackageDeclaration** pout) {
  JavaPackageDeclaration package_declaration;
  package_declaration.fragment.begin = parser->pos;
  package_declaration.package_name = NULL;
  
  gboolean success = FALSE;
  java_parse_whitespace_multiline(parser);
  if (java_parse_literal(parser, "package")) {
    java_parse_whitespace_multiline(parser);
    if (java_parse_qualified_name(parser, &(package_declaration.package_name))) {
      java_parse_whitespace_multiline(parser);
      if (java_parse_literal(parser, ";")) {
        java_parse_whitespace_singleline(parser);
        success = TRUE;
      } else {
        // TODO: Handle unexpected format (missing trailing ';').
      }
    } else {
      // TODO: Handle unexpected format (missing qualified name).
    }
  }
  java_parser_update_fragment(success, parser, &package_declaration.fragment);
  
  if (success && pout) {
    *pout = STRUCT_DUP(JavaPackageDeclaration, &package_declaration);
  } else {
    java_package_declaration_destroy(&package_declaration);
  }
  return success;
}

void java_package_declaration_free(JavaPackageDeclaration* package_declaration) {
  java_package_declaration_destroy(package_declaration);
  if (package_declaration) {
    g_free(package_declaration);
  }
}

static void java_package_declaration_destroy(JavaPackageDeclaration* package_declaration) {
  if (package_declaration && package_declaration->package_name) {
    g_free(package_declaration->package_name);
  }
}
