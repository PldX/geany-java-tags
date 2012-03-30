#ifndef JAVA_PARSER_IMPL_H_
#define JAVA_PARSER_IMPL_H_

#include "java-parser.h"
#include "text.h"

typedef struct _JavaParser {
  JavaParseOptions* opts;
  Text* text;
  unsigned int pos;
  gboolean err;
} JavaParser;

////////////////////////////////////////////////////////////////////////////////
// common
////////////////////////////////////////////////////////////////////////////////

// Parses the eof.
gboolean java_parse_eof(JavaParser* parser);
// Parses the eol.
gboolean java_parse_eol(JavaParser* parser);
// Parses exact literal.
gboolean java_parse_literal(JavaParser* parser, const char* literal);
// Parser a single java identifier.
gboolean java_parse_identifier(JavaParser* parser, char** pout);
// Parses a (possibly) qualified type name.
gboolean java_parse_qualified_name(JavaParser* parser, char** pout);

// Utility for updating parsed fragment after parse:
// * on success, sets the fragment end from current parser position
// * on failrue, resets the parser position to the fragment begin
void java_parser_update_fragment(gboolean success, JavaParser* parser, JavaTextFragment* fragment);

////////////////////////////////////////////////////////////////////////////////
// whitespace and comments
////////////////////////////////////////////////////////////////////////////////

// Parses white-space and comments accross multiple lines.
void java_parse_whitespace_multiline(JavaParser* parser);
// Parser white-space and comments and the end-of-line at most once.
// Exception: multi-line comments in white-space can spawn over multiple lines.
void java_parse_whitespace_singleline(JavaParser* parser);

////////////////////////////////////////////////////////////////////////////////
// package
////////////////////////////////////////////////////////////////////////////////

// Parses a package declaration.
gboolean java_parse_package(JavaParser* parser, JavaPackageDeclaration** pout);
// Deletes a java package declaration.
void java_package_declaration_free(JavaPackageDeclaration* package_declaration);

////////////////////////////////////////////////////////////////////////////////
// import
////////////////////////////////////////////////////////////////////////////////

// Parses an import declaration.
gboolean java_parse_import(JavaParser* parser, JavaImportDeclaration** pout);
// Deletes a java import declaration.
void java_import_declaration_free(JavaImportDeclaration* import_declaration);

////////////////////////////////////////////////////////////////////////////////
// type
////////////////////////////////////////////////////////////////////////////////

// Parses a java type declaration.
gboolean java_parse_type(JavaParser* parser, JavaTypeDeclaration** pout);
// Deletes a java type declaration.
void java_type_declaration_free(JavaTypeDeclaration* type_declaration);

#endif // JAVA_PARSER_IMPL_H_
