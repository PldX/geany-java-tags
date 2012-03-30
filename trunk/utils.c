#include "utils.h"

gint array_len(const gpointer* pp) {
  const gpointer* ppp;
  for (ppp = pp; *ppp; ++ppp) {
    // Empty.
  }
  return (ppp - pp);
}

gpointer* array_insert(gpointer* pp, gint index, gpointer p) {
  gint len = array_len(pp);
  if (index > len) {
    index = len;
  }
  pp = g_renew(gpointer, pp, len + 2);
  gint i;
  for (i = len+1; i > index; --i) {
    pp[i] = pp[i-1];
  }
  pp[index] = p;
  return pp;
}

gpointer* array_add(gpointer* pp, gpointer p) {
  gint len = array_len(pp);
  pp = g_renew(gpointer, pp, len + 2);
  pp[len] = p;
  pp[len+1] = NULL;
  return pp;
}

void array_free(gpointer* pp, gpointerfree pfree) {
  if (pp) {
    gpointer* ppp;
    for (ppp = pp; *ppp; ++ppp) {
      pfree(*ppp);
    }
    g_free(pp);
  }
}

gchar* g_stradd(gchar* dst, const gchar* src) {
  gchar* new_dst = g_strconcat(dst, src, NULL);
  g_free(dst);
  return new_dst;
}
