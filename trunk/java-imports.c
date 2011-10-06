#include "java-imports.h"
#include "sci-utils.h"
#include "utils.h"

GeanyFunctions* geany_functions;  // Required by sci* methods

/**
 * An import group is a list of consecutive import lines that matches the same
 * regular expression.
 */
typedef struct _ImportGroup {
  // Index of the first line number in the document of the import group.
  gint line;
  // The content of the import group.
  gchar** imports;
  // The index of the matched regexp.
  gint order;
} ImportGroup;

static gint java_import_groups_get_order(const gchar* import, const GRegex** order);
static ImportGroup* java_import_group_new(gint order, gint line);
static ImportGroup** java_import_groups_parse(ScintillaObject* sci, const GRegex** order_regex);
static void java_import_group_insert(ImportGroup* import_group, const gchar* import);
static ImportGroup** java_import_groups_ensure_order(ImportGroup** import_groups, gint order);
static void java_import_group_write(ImportGroup* import_group, ScintillaObject* sci);
static void java_import_groups_free(ImportGroup** import_groups);

void java_import(const gchar* import, const gchar** import_groups_order, ScintillaObject* sci) {
  GRegex** pr;
  GRegex** regexes = g_new(GRegex*, array_len((const gpointer*) import_groups_order) + 1);
  gint i;
  for (pr = regexes, i = 0; import_groups_order[i]; ++i) {
    gchar* regex_str = g_strstrip(g_strdup(import_groups_order[i]));
    if (*regex_str) {
      *pr = g_regex_new(regex_str, 0, G_REGEX_MATCH_ANCHORED, NULL);
      if (*pr) {
        ++pr;
      } else {
        g_warning("Invalid regex: %s", import_groups_order[i]);
      }
    }
    g_free(regex_str);
  }
  *pr = NULL;
  gint order = java_import_groups_get_order(import, (const GRegex**) regexes);
  if (order >= 0) {
    ImportGroup** import_groups = java_import_groups_parse(sci, (const GRegex**) regexes);
    import_groups = java_import_groups_ensure_order(import_groups, order);
    ImportGroup* import_group = NULL;
    ImportGroup** pig;
    for (pig = import_groups; *pig; ++pig) {
      if ((*pig)->order == order) {
        import_group = *pig;
        break;
      }
    }
    java_import_group_insert(import_group, import);
    java_import_group_write(import_group, sci);
    java_import_groups_free(import_groups);
  }
  for (pr = regexes; *pr; ++pr) {
    g_regex_unref(*pr);
  }
  g_free(regexes);
}

static ImportGroup* java_import_group_new(gint order, gint line) {
  ImportGroup* import_group = g_new(ImportGroup, 1);
  import_group->order = order;
  import_group->line = line;
  import_group->imports = g_new0(gchar*, 1);
  return import_group;
}

static gint java_import_groups_get_order(const gchar* import, const GRegex** regexes) {
  const GRegex** pr;
  for (pr = regexes; *pr; ++pr) {
    if (g_regex_match(*pr, import, G_REGEX_MATCH_ANCHORED, NULL)) {
      return (pr - regexes);
    }
  }
  return -1;
}

static ImportGroup** java_import_groups_parse(ScintillaObject* sci, const GRegex** regexes) {
  ImportGroup** import_groups = g_new0(ImportGroup*, 1);
  ImportGroup* import_group = NULL;
  gint i, n = sci_get_line_count(sci);
  for (i = 0; i < n; ++i) {
    gchar* line = sci_get_line(sci, i);
    gint order = java_import_groups_get_order(line, regexes);
    if (import_group != NULL && import_group->order != order) {
      import_group = NULL;
    }
    if (import_group == NULL && order != -1) {
      import_group = java_import_group_new(order, i);
      import_groups = (ImportGroup**) array_add(
          (gpointer*) import_groups, (gpointer) import_group);
    }
    if (import_group != NULL) {
      import_group->imports = (gchar**) array_add(
          (gpointer*) (import_group->imports), (gpointer) g_strdup(line));
    }
    g_free(line);
  }
  return import_groups;
}

static void java_import_group_insert(ImportGroup* import_group, const gchar* import) {
  gchar** new_imports = g_new0(gchar*, array_len((gpointer*)(import_group->imports)) + 2);
  gchar** pi;
  gchar** pni = new_imports;
  for (pi = import_group->imports; *pi; ++pi) {
    if (g_strcmp0(import, *pi) < 0) {
      break;
    }
    *pni = *pi;
    ++pni;
  }
  if (!*pi || g_strcmp0(import, *pi)) {
    *pni = g_strdup(import); // Only the new import is dupped.
    ++pni;
  }
  for (; *pi; ++pi) {
    *pni = *pi;
    ++pni;
  }
  g_free(import_group->imports); // Existing imports are moved by reference.
  import_group->imports = new_imports;
}

static ImportGroup** java_import_groups_ensure_order(ImportGroup** import_groups, gint order) {
  gint line = 0;
  ImportGroup** pig;
  for (pig = import_groups; *pig; ++pig) {
    if ((*pig)->order == order) {
      return import_groups;
    } else if ((*pig)->order < order) {
      line = (*pig)->line;
      break;
    }
    line = (*pig)->line + array_len((gpointer*)((*pig)->imports));
  }
  ImportGroup* import_group = java_import_group_new(order, line);
  return (ImportGroup**) array_insert(
      (gpointer*) import_groups, pig-import_groups,
      (gpointer) import_group);
}

static void java_import_group_write(ImportGroup* import_group, ScintillaObject* sci) {
  gint i;
  gchar** import;
  for (i = import_group->line, import = import_group->imports; *import; ++import, ++i) {
    g_message("Attempt to write line %i: %s", i, *import);
    gchar* line = sci_get_line(sci, i);
    if (g_strcmp0(line, *import) != 0) {
      sci_insert_text(sci, sci_get_position_from_line(sci, i), *import);
    }
    g_free(line);
  }
}

static void java_import_groups_free(ImportGroup** import_groups) {
  ImportGroup** pig;
  for (pig = import_groups; *pig; ++pig) {
    g_strfreev((*pig)->imports);
    g_free(*pig);
  }
  g_free(import_groups);
}
