#include "sci-text.h"

#include <glib.h>
#include <string.h>

#include "sci-utils.h"

GeanyFunctions* geany_functions;

static char sci_text_get_char(ScintillaObject* sci, unsigned int i);
static char* sci_text_get_range(ScintillaObject* sci, unsigned int i, int j);
static void sci_text_set_range(ScintillaObject* sci, unsigned int i, int j, const char* src);
static int sci_text_is_eof(ScintillaObject* sci, unsigned int i);
static void sci_text_free(ScintillaObject* sci);

Text* sci_text_new(ScintillaObject* sci) {
  return text_new(sci,
                  (VirtualTextGetChar) sci_text_get_char,
                  (VirtualTextGetRange) sci_text_get_range,
                  (VirtualTextSetRange) sci_text_set_range,
                  (VirtualTextIsEof) sci_text_is_eof,
                  (VirtualTextFree) sci_text_free);
}

static char sci_text_get_char(ScintillaObject* sci, unsigned int i) {
  return (i >= sci_get_length(sci)) ? 0 : sci_get_char_at(sci, i);
}

static char* sci_text_get_range(ScintillaObject* sci, unsigned int i, int n) {
  unsigned int length = (unsigned int) sci_get_length(sci);
  if (i > length) {
    i = length;
  }
  if (n < 0) {
    n = length;
  }
  if (i + n > length) {
    n = length - i;
  }
  return sci_get_contents_range(sci, i, i + n);
}

static void sci_text_set_range(ScintillaObject* sci, unsigned int i, int n, const char* src) {
  unsigned int length = sci_get_length(sci);
  if (i > length) {
    i = length;
  }
  if (n < 0) {
    n = length;
  }
  if (i + n > length) {
    n = length - i;
  }
  sci_replace_text(sci, i, i+n, src);
}

static int sci_text_is_eof(ScintillaObject* sci, unsigned int i) {
  return i >= sci_get_length(sci);
}

static void sci_text_free(ScintillaObject* sci) {
  // Empty.
}

