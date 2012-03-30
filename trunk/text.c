#include "text.h"

#include <stdlib.h>
#include <glib.h>

Text* text_new(void* data, 
               VirtualTextGetChar get_char_impl,
               VirtualTextGetRange get_range_impl,
               VirtualTextSetRange set_range_impl,
               VirtualTextIsEof is_eof_impl,
               VirtualTextFree free_impl) {
  Text* text = (Text*) malloc(sizeof(Text));
  text->data = data;
  text->get_char_impl = get_char_impl;
  text->get_range_impl = get_range_impl;
  text->set_range_impl = set_range_impl;
  text->is_eof_impl = is_eof_impl;
  text->free_impl = free_impl;
  return text;
}

char text_get_char(Text* text, unsigned int pos) {
  char ch = (text->get_char_impl)(text->data, pos);
  // g_message("Reading: %c", ch);
  return ch;
}

char* text_get_range(Text* text, unsigned int pos, int length) {
  return (text->get_range_impl)(text->data, pos, length);
}

void text_set_range(Text* text, unsigned int pos, int length, const char* fragment) {
  return (text->set_range_impl)(text->data, pos, length, fragment);
}

int text_is_eof(Text* text, unsigned int pos) {
  return (text->is_eof_impl)(text->data, pos);
}

void text_free(Text* text) {
  (text->free_impl)(text->data);
  free(text);
}
