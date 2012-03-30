#include "java-parser-impl.h"

#include "java-parser.h"

void java_parser_update_fragment(gboolean success, 
                                 JavaParser* parser, 
                                 JavaTextFragment* fragment) {
  if (success) {
    fragment->end = parser->pos;
  } else {
    parser->pos = fragment->begin;
  }
}


gboolean java_parse_eof(JavaParser* parser) {
  return text_is_eof(parser->text, parser->pos);
}

gboolean java_parse_literal(JavaParser* parser, const char* literal) {
  unsigned int mark = parser->pos;
  for (; *literal; ++literal, ++(parser->pos)) {
    char ch = text_get_char(parser->text, parser->pos);
    if (ch != *literal) {
      parser->pos = mark;
      return FALSE;
    }
  }
  return TRUE;
}

static gboolean java_parse_java_letter(JavaParser* parser) {
  char ch = text_get_char(parser->text, parser->pos);
  if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z') {
    ++(parser->pos);
    return TRUE;
  }
    g_message("Not java letter: %c", ch);
  return FALSE;
}
static gboolean java_parse_java_letter_or_digit(JavaParser* parser) {
  char ch = text_get_char(parser->text, parser->pos);
  if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9') {
    ++(parser->pos);
    return TRUE;
  }
  return FALSE;
}

gboolean java_parse_identifier(JavaParser* parser, char** pout) {
  unsigned int begin = parser->pos;
  if (!java_parse_java_letter(parser)) {
    return FALSE;
  }
  while (java_parse_java_letter_or_digit(parser)) {
    // Empty.
  }
  if (pout) {
    *pout = text_get_range(parser->text, begin, parser->pos - begin);
  }
  return TRUE;
}

gboolean java_parse_qualified_name(JavaParser* parser, char** pout) {
  unsigned int begin = parser->pos;
  if (!java_parse_identifier(parser, NULL)) {
    return FALSE;
  }
  while (java_parse_literal(parser, ".")) {
    if (!java_parse_identifier(parser, NULL)) {
      parser->pos = begin;
      return FALSE;
    }
  }
  if (pout) {
    *pout = text_get_range(parser->text, begin, parser->pos - begin);
  }
  return TRUE;
}

gboolean java_parse_eol(JavaParser* parser) {
  char ch = text_get_char(parser->text, parser->pos);
  if (ch == '\n') {
    // LF
    ++parser->pos;
    return TRUE;
  }
  if (ch == '\r') {
    // CR
    ++parser->pos;
    ch = text_get_char(parser->text, parser->pos);
    if (ch == '\n') {
      // CR LF
      ++parser->pos;
    }
    return TRUE;
  }
  return FALSE;
}

