#include "java-parser.h"
#include "java-parser-impl.h"

#include <string.h>

// Destroys the content of a type declaration without deleting the pointer.
static void java_type_declaration_destroy(JavaTypeDeclaration* type_declaration);

gboolean java_parse_type(JavaParser* parser, JavaTypeDeclaration** pout) {
  return FALSE;
}

void java_type_declaration_free(JavaTypeDeclaration* type_declaration) {
  java_type_declaration_destroy(type_declaration);
  if (type_declaration) {
    g_free(type_declaration);
  }
}

static void java_type_declaration_destroy(JavaTypeDeclaration* type_declaration) {
}
