#ifndef GEANY_JAVA_TAGS_UTILS_H_
#define GEANY_JAVA_TAGS_UTILS_H_

#include <glib.h>

gint array_len(const gpointer* pp);
gpointer* array_insert(gpointer* pp, gint index, gpointer p);
gpointer* array_add(gpointer* pp, gpointer p);

#endif // GEANY_JAVA_TAGS_UTILS_H_
