#include "sci-utils.h"
#include <string.h>

GeanyFunctions* geany_functions;  // Required by sci* methods

int is_word_char(gchar ch) {
  return isalnum(ch) || ch == '_';
}

void sci_get_current_word(ScintillaObject* sci, gint* pstart, gint* pend) {
  gint pos = sci_get_current_position(sci);
  gint start = pos-1;
  while (start >= 0 && is_word_char(sci_get_char_at(sci, start))) {
    --start;
  }
  ++start;
  gint end = pos;
  gint n = sci_get_length(sci);
  while (end < n && is_word_char(sci_get_char_at(sci, end))) {
    ++end;
  }
  *pstart = start;
  *pend = end;
}

gchar* sci_get_current_word_text(ScintillaObject* sci) {
  if (!sci) {
    return NULL;
  }
  gint start;
  gint end;
  sci_get_current_word(sci, &start, &end);
  if (start >= end) {
    return NULL;
  }
  return sci_get_contents_range(sci, start, end);
}

void sci_replace_text(ScintillaObject* sci, gint start, gint end, gchar* text) {
  gint sel_start = sci_get_selection_start(sci);
  gint sel_end = sci_get_selection_end(sci);
  
  sci_set_selection_start(sci, start);
  sci_set_selection_end(sci, end);
  sci_replace_sel(sci, text);
  
  gint sel_delta = strlen(text) - (start - end);
  if (sel_start > end) {
    sel_start += sel_delta;
  }
  if (sel_end > end) {
    sel_end += sel_delta;
  }
  sci_set_selection_start(sci, sel_start);
  sci_set_selection_end(sci, sel_end);
}

void sci_replace_current_word(ScintillaObject* sci, gchar* word) {
  if (!sci) {
    return;
  }
  gint start;
  gint end;
  sci_get_current_word(sci, &start, &end);
  if (start > end) {
    start = end;
  }
  sci_replace_text(sci, start, end, word);
}
