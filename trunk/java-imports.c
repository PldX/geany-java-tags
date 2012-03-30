#include "java-imports.h"

#include <string.h>

#include "java-parser.h"
#include "text.h"
#include "utils.h"

// Finds the group index for an import declaration relative to user specified
// import groups order.
//
// import_groups_order is complete: an implicit match all is assumed at the end.
// Returned index is never negative, but posibly equal to len(import_groups_order).
static unsigned int java_import_group_index(
    const gchar** import_groups_order,
    const JavaImportDeclaration* import);
// Various variants that iterates over a null-terminated array of import declarations
// to find the first matching import.
// Returns a reference to the first matching import declaration or NULL if not found.
static const JavaImportDeclaration** java_import_first_import_same_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index);
static const JavaImportDeclaration** java_import_next_import_same_group(
    const JavaImportDeclaration** imports,
    const JavaImportDeclaration* import,
    const gchar** import_groups_order,
    unsigned int import_group_index);
static const JavaImportDeclaration** java_import_last_import_same_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index);
static const JavaImportDeclaration** java_import_first_import_next_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index);
static const JavaImportDeclaration** java_import_last_import(
    const JavaImportDeclaration** imports);

static enum JavaImportOptions {
  // Inserts the import after specified position.
  INSERT_AFTER = 0x01,
  // Inserts the import before specified position.
  INSERT_BEFORE = 0x02,
  // Inserts an empty line after the new import.
  INSERT_LINE_AFTER = 0x04,
  // Inserts an empty line before the new import.
  INSERT_LINE_BEFORE = 0x08,
  // Skips empty lines in text before insert.
  SKIP_EMPTY_LINES = 0x10,
};
// Actual text insert method that formats the text to be output.
// Accepts variations through bit-style options.
static void java_import_insert(const JavaImportDeclaration* import,
                               Text* text, unsigned int pos,
                               unsigned int options);
void java_import(const gchar* type_name, const gchar** import_groups_order,
                 Text* sci_text) {
  // Find the group order for imported type name.
  JavaImportDeclaration import;
  import.static_import = FALSE;
  import.on_demand = FALSE;
  import.type_name = (char*) type_name;
  unsigned int import_group_index = java_import_group_index(
      import_groups_order,
      &import);

  // Parse imports in the document.
  JavaParseOptions parse_opts;
  java_parse_opts_init(&parse_opts);
  parse_opts.extract_package = TRUE;
  parse_opts.extract_imports = TRUE;
  JavaDocStructure* java_doc = java_parse(sci_text, &parse_opts);
  if (java_doc) {
    // Fint first existing import in the import group where the import should be inserted.
    const JavaImportDeclaration** first_import_same_group = java_import_first_import_same_group(
        (const JavaImportDeclaration**) java_doc->import_declarations,
        import_groups_order,
        import_group_index);
    // Insert import in the existing import group.
    if (first_import_same_group) {
      // Find insert position import in the import group where the import should be inserted.
      const JavaImportDeclaration** next_import_same_group = java_import_next_import_same_group(
          first_import_same_group,
          &import,
          import_groups_order,
          import_group_index);
      if (next_import_same_group) {
        // Insert before next import in alphabetic order.
        java_import_insert(&import,
                           sci_text, (*next_import_same_group)->fragment.begin,
                           SKIP_EMPTY_LINES | INSERT_BEFORE);
      } else {
        // Find last import in the import group where the import should be inserted.
        const JavaImportDeclaration** last_import_same_group = java_import_last_import_same_group(
            first_import_same_group,
            import_groups_order,
            import_group_index);
        // Insert after last import in group.
        java_import_insert(&import,
                           sci_text, (*last_import_same_group)->fragment.end,
                           INSERT_AFTER);
      }
    // Insert import as a new import group.
    } else {
      // Find next import group.
      const JavaImportDeclaration** first_import_next_group = java_import_first_import_next_group(
          (const JavaImportDeclaration**) java_doc->import_declarations,
          import_groups_order,
          import_group_index);
      if (first_import_next_group) {
        // Insert before next import group.
        java_import_insert(&import,
                           sci_text, (*first_import_next_group)->fragment.begin,
                           INSERT_BEFORE | INSERT_LINE_AFTER | SKIP_EMPTY_LINES);
      } else {
        // Find last absolute import.
        const JavaImportDeclaration** last_import = java_import_last_import(
            (const JavaImportDeclaration**) java_doc->import_declarations);
        if (last_import) {
          // Insert after last import.
          java_import_insert(&import,
                             sci_text, (*last_import)->fragment.end,
                             INSERT_AFTER | INSERT_LINE_BEFORE);
        } else if (java_doc->package_declaration) {
          // Insert after package declaration.
          java_import_insert(&import,
                             sci_text, java_doc->package_declaration->fragment.end,
                             INSERT_AFTER | INSERT_LINE_BEFORE);
        } else {
          // Insert at the begining of the file; don't skip comments because it is
          // ambiguous whether they are file header or belongs to the first type in
          // the file.
          java_import_insert(&import,
                             sci_text, 0,
                             INSERT_AFTER | INSERT_LINE_AFTER);
        }
      }
    }
  }
  java_doc_free(java_doc);
}

static unsigned int java_import_group_index(
    const gchar** import_groups_order,
    const JavaImportDeclaration* import) {
  int index = -1;
  int index_score = 0;
  int i;
  for (i = 0; import_groups_order[i]; ++i) {
    gboolean static_match = FALSE;
    const gchar* import_group_prefix = import_groups_order[i];
    if (g_str_has_prefix(import_group_prefix, "static:")) {
      static_match = TRUE;
      import_group_prefix += strlen("static:");
    }
    int score = 0;
    if (static_match) {
      score += import->static_import ? 100 : -100;
    }
    if (g_str_has_prefix(import->type_name, import_group_prefix)) {
      score += strlen(import_group_prefix);
    }
    if (score > index_score) {
      index = i;
      index_score = score;
    }
  }
  if (index == -1) {
    index = i;
  }
  return index;
}

static gboolean same_group(const JavaImportDeclaration* import,
                           const gchar** import_groups_order,
                           unsigned int import_group_index) {
  return java_import_group_index(import_groups_order, import) == import_group_index;
}

static const JavaImportDeclaration** java_import_first_import_same_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index) {
  const JavaImportDeclaration** import_ptr = NULL;
  int i;
  for (i = 0; !import_ptr && imports[i]; ++i) {
    if (same_group(imports[i], import_groups_order, import_group_index)) {
      import_ptr = imports + i;
    }
  }
  return import_ptr;
}

static const JavaImportDeclaration** java_import_next_import_same_group(
    const JavaImportDeclaration** imports,
    const JavaImportDeclaration* import,
    const gchar** import_groups_order,
    unsigned int import_group_index) {
  const JavaImportDeclaration** import_ptr = NULL;
  int i;
  for (i = 0; !import_ptr && imports[i] && same_group(imports[i], import_groups_order, import_group_index); ++i) {
    if (g_strcmp0(import->type_name, /* <= */ imports[i]->type_name) <= 0) {
      // First in imports bigger then import.
      import_ptr = imports + i;
    }
  }
  return import_ptr;
}

static const JavaImportDeclaration** java_import_last_import_same_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index) {
  const JavaImportDeclaration** import_ptr = imports;
  int i;
  for (i = 0; imports[i] && same_group(imports[i], import_groups_order, import_group_index); ++i) {
    import_ptr = imports + i;
  }
  return import_ptr;
}

static const JavaImportDeclaration** java_import_first_import_next_group(
    const JavaImportDeclaration** imports,
    const gchar** import_groups_order,
    unsigned int import_group_index) {
  const JavaImportDeclaration** import_ptr = NULL;
  int i;
  for (i = 0; !import_ptr && imports[i]; ++i) {
    if (java_import_group_index(import_groups_order, imports[i]) > import_group_index) {
      import_ptr = imports + i;
    }
  }
  return import_ptr;
}

static const JavaImportDeclaration** java_import_last_import(
    const JavaImportDeclaration** imports) {
  const JavaImportDeclaration** import_ptr = NULL;
  int i;
  for (i = 0; imports[i]; ++i) {
    import_ptr = imports + i;
  }
  return import_ptr;
}

static unsigned int skip_empty_lines(Text* text, unsigned int pos) {
  char ch = text_get_char(text, pos);
  if (ch == '\n') {
    // LF
    ++pos;
  } else if (ch == '\r') {
    // CR
    ++pos;
    ch = text_get_char(text, pos);
    if (ch == '\n') {
      // CR LF
      ++pos;
    }
  }
  return pos;
}

static void java_import_insert(const JavaImportDeclaration* import,
                               Text* text, unsigned int pos,
                               unsigned int options) {
  if (options & SKIP_EMPTY_LINES) {
    pos = skip_empty_lines(text, pos);
  }

  char* import_text = g_strdup("");
  if (options & INSERT_LINE_BEFORE) {
    import_text = g_stradd(import_text, "\n");
  }
  import_text = g_stradd(import_text, "import ");
  if (import->static_import) {
    import_text = g_stradd(import_text, "static ");
  }
  import_text = g_stradd(import_text, import->type_name);
  if (import->on_demand) {
    import_text = g_stradd(import_text, ".*");
  }
  import_text = g_stradd(import_text, ";\n");
  if (options & INSERT_LINE_AFTER) {
    import_text = g_stradd(import_text, "\n");
  }

  text_set_range(text, pos, 0, import_text);

  g_free(import_text);
}
