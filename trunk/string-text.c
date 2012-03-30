#include "string-text.h"

#include <glib.h>
#include <string.h>

#include "utils.h"

typedef struct _StringText {
  char* text;
  int length;
} StringText;

static char string_text_get_char(StringText* text, unsigned int i);
static char* string_text_get_range(StringText* text, unsigned int i, int j);
static void string_text_set_range(StringText* text, unsigned int i, int j, const char* src);
static int string_text_is_eof(StringText* text_data, unsigned int i);
static void string_text_free(StringText* text);

Text* string_text_new(const char* initial) {
  StringText* text = g_new(StringText, 1);
  text->text = g_strdup(initial);
  text->length = strlen(initial);
  return text_new(text,
                  (VirtualTextGetChar) string_text_get_char,
                  (VirtualTextGetRange) string_text_get_range,
                  (VirtualTextSetRange) string_text_set_range,
                  (VirtualTextIsEof) string_text_is_eof,
                  (VirtualTextFree) string_text_free);
}

char* string_text_get(Text* text) {
  return g_strdup(((StringText*) text->data)->text);
}

static char string_text_get_char(StringText* text, unsigned int i) {
  return i < text->length ? text->text[i] : '\0';
}

static char* string_text_get_range(StringText* text, unsigned int i, int n) {
  if (i > text->length) {
    i = text->length;
  }
  if (n < 0) {
    n = text->length;
  }
  if (i + n > text->length) {
    n = text->length - i;
  }
  return g_strndup(text->text + i, n);
}

static void string_text_set_range(StringText* text, unsigned int i, int n, const char* src) {
  if (i < 0) {
    i = 0;
  }
  if (i > text->length) {
    i = text->length;
  }
  if (n < 0) {
    n = text->length;
  }
  if (i + n > text->length) {
    n = text->length - i;
  }
  char* dst = g_strndup(text->text, i);
  dst = g_stradd(dst, src);
  dst = g_stradd(dst, text->text + i + n);
  g_free(text->text);
  text->text = dst;
  text->length = strlen(dst);
}

static int string_text_is_eof(StringText* text, unsigned int i) {
  return i >= text->length;
}

static void string_text_free(StringText* text) {
  g_free(text->text);
  g_free(text);
}

