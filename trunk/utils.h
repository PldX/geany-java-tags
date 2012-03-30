#ifndef GEANY_JAVA_TAGS_UTILS_H_
#define GEANY_JAVA_TAGS_UTILS_H_

#include <glib.h>

// Gets the length of a null-terminated pointer array.
gint array_len(const gpointer* pp);
// Inserts a pointer into an array of pointers.
gpointer* array_insert(gpointer* pp, gint index, gpointer p);
// Appends a pointer to a null-terminated array of pointers.
gpointer* array_add(gpointer* pp, gpointer p);
// Frees an array and its components.
typedef void (*gpointerfree)(gpointer);
void array_free(gpointer* pp, gpointerfree pfree);

#define ARRAY_LEN(p) array_len((gpointer*) (p));
#define ARRAY_ADD(pp, p) array_add((gpointer*) (pp), (gpointer) p);
#define ARRAY_FREE(pp, pfree) array_free((gpointer*) (pp), (gpointerfree) (pfree));

#define STRUCT_DUP(type, src) (type*) g_memdup((gpointer) (src), sizeof(type));

gchar* g_stradd(gchar* dst, const gchar* src);

#endif // GEANY_JAVA_TAGS_UTILS_H_
