#ifndef JAVA_PARSER_H_
#define JAVA_PARSER_H_

#include <glib.h>

struct _Text;

typedef struct _JavaParseOptions {
  gboolean extract_package;
  gboolean extract_imports;
  gboolean extract_types;
  gboolean extract_members;
  gboolean extract_body;
} JavaParseOptions;

typedef struct _JavaTextFragment {
  unsigned int begin;
  unsigned int end;
} JavaTextFragment;

typedef struct _JavaPackageDeclaration {
  JavaTextFragment fragment;
  char* package_name;
} JavaPackageDeclaration;

typedef struct _JavaImportDeclaration {
  JavaTextFragment fragment;
  gboolean static_import;
  gboolean on_demand;
  char* type_name;
} JavaImportDeclaration;

typedef enum _JavaVisibility {
  JAVA_VISIBILITY_PUBLIC,
  JAVA_VISIBILITY_PROTECTED,
  JAVA_VISIBILITY_PRIVATE,
  JAVA_VISIBILITY_PACKAGE,
} JavaVisibility;
typedef struct _JavaFieldDeclaration {
  JavaTextFragment fragment;
  JavaVisibility visibility;
  char* type;
  char* name;
} JavaFieldDeclaration;
typedef struct _JavaMethodArgument {
  JavaTextFragment fragment;
  char* type;
  char* name;
} JavaMethodArgument;
typedef struct _JavaMethodDeclaration {
  JavaTextFragment fragment;
  JavaVisibility visibility;
  char* return_type;
  char* name;
  JavaMethodArgument** arguments;
} JavaMethodDeclaration;
typedef struct _JavaTypeDeclaration {
  JavaTextFragment fragment;
  JavaVisibility visibility;
  char* clazz;
  char* super_class;
  char** interfaces;
  struct _JavaTypeDeclaration** nested_types;
  JavaFieldDeclaration** fields;
  JavaMethodDeclaration** methods;
} JavaTypeDeclaration;

typedef struct _JavaDocStructure {
  JavaPackageDeclaration* package_declaration;
  JavaImportDeclaration** import_declarations;
  JavaTypeDeclaration** type_declarations;
} JavaDocStructure;

void java_parse_opts_init(JavaParseOptions* opts);
// Parses a text into a structured java document.
JavaDocStructure* java_parse(struct _Text* text, JavaParseOptions* opts);
// Frees a structured java document parsed by java_parse.
void java_doc_free(JavaDocStructure* doc);

#endif // JAVA_PARSER_H_
