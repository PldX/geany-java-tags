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
  return array_insert(pp, array_len(pp), p);
}

