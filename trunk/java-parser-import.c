#include "java-parser.h"
#include "java-parser-impl.h"

#include <glib.h>
#include <string.h>

#include "utils.h"

// Parses the import type in a import type declaration.
static gboolean java_parse_import_type(JavaParser* parser, char** pout, gboolean* on_demand);
// Destroys the content of a import declaration without deleting the pointer.
static void java_import_declaration_destroy(JavaImportDeclaration* import_declaration);

gboolean java_parse_import(JavaParser* parser, JavaImportDeclaration** pout) {
  JavaImportDeclaration import_declaration;
  import_declaration.fragment.begin = parser->pos;
  import_declaration.static_import = FALSE;
  import_declaration.on_demand = FALSE;
  import_declaration.type_name = NULL;

  gboolean success = FALSE;
  java_parse_whitespace_multiline(parser);
  if (java_parse_literal(parser, "import")) {
    java_parse_whitespace_multiline(parser);
    if (java_parse_literal(parser, "static")) {
      import_declaration.static_import = TRUE;
      java_parse_whitespace_multiline(parser);
    }
    if (java_parse_import_type(parser, &import_declaration.type_name,
                               &import_declaration.on_demand)) {
      java_parse_whitespace_multiline(parser);
      if (java_parse_literal(parser, ";")) {
        java_parse_whitespace_singleline(parser);
        success = TRUE;
      } else {
        // TODO: Handle unexpected format (missing trailing ';').
      }
    } else {
      // TODO: Handle unexpected format (invalid import type).
    }
  }
  java_parser_update_fragment(success, parser, &import_declaration.fragment);

  if (success && pout) {
    *pout = STRUCT_DUP(JavaImportDeclaration, &import_declaration);
  } else {
    java_import_declaration_destroy(&import_declaration);
  }
  return success;
}

void java_import_declaration_free(JavaImportDeclaration* import_declaration) {
  java_import_declaration_destroy(import_declaration);
  if (import_declaration) {
    g_free(import_declaration);
  }
}

static gboolean java_parse_import_type(JavaParser* parser, char** pout, gboolean* on_demand) {
  unsigned int begin = parser->pos;
  unsigned int end = begin;

  gboolean success = TRUE;
  if (java_parse_identifier(parser, NULL)) {
    end = parser->pos;
    *on_demand = FALSE;
    while (success && java_parse_literal(parser, ".")) {
      if (java_parse_literal(parser, "*")) {
        *on_demand = TRUE;
        break;
      } else if (java_parse_identifier(parser, NULL)) {
        end = parser->pos;
      } else {
        success = FALSE;
      }
    }
  } else {
    success = FALSE;
  }
  if (!success) {
    parser->pos = begin;
  }

  if (success && pout) {
    *pout = text_get_range(parser->text, begin, end - begin);
  }
  return success;
}

static void java_import_declaration_destroy(JavaImportDeclaration* import_declaration) {
  if (import_declaration && import_declaration->type_name) {
    g_free(import_declaration->type_name);
  }
}
