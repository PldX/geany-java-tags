#include "java-parser-impl.h"

gboolean java_parse_traditional_comment(JavaParser* parser) {
  if (!java_parse_literal(parser, "/*")) {
    return FALSE;
  }
  // Java grammar doesn't specify escaping in traditional comments.
  for (; !java_parse_literal(parser, "*/"); ++parser->pos) {
    if (text_is_eof(parser->text, parser->pos)) {
      parser->err = TRUE; // TODO: Exception.
      return FALSE;
    }
  }
  return TRUE;
}

// Includes the eol.
gboolean java_parse_eol_comment(JavaParser* parser) {
  if (!java_parse_literal(parser, "//")) {
    return FALSE;
  }
  for (; !java_parse_eol(parser); ++parser->pos) {
    if (text_is_eof(parser->text, parser->pos)) {
      break;
    }
  }
  return TRUE;
}

// Parses traditional or eol comment.
gboolean java_parse_comment(JavaParser* parser) {
  return java_parse_traditional_comment(parser) || java_parse_eol_comment(parser);
}

gboolean is_whitespace(char ch) {
  return ch == '\t' || ch == ' ';
}

gboolean java_parse_whitespace(JavaParser* parser) {
  unsigned int begin = parser->pos;
  while (TRUE) {
    char ch = text_get_char(parser->text, parser->pos);
    if (is_whitespace(ch)) {
      ++parser->pos;
      continue;
    }
    if (ch == '/' && java_parse_comment(parser)) {
      continue;
    }
    break;
  }
  return parser->pos != begin;
}

void java_parse_whitespace_multiline(JavaParser* parser) {
  while (java_parse_whitespace(parser) || java_parse_eol(parser)) {
    if (text_is_eof(parser->text, parser->pos)) {
      break;
    }
  }
}

void java_parse_whitespace_singleline(JavaParser* parser) {
  unsigned int begin = parser->pos;
  while (TRUE) {
    char ch = text_get_char(parser->text, parser->pos);
    if (is_whitespace(ch)) {
      ++parser->pos;
      continue;
    }
    if (ch == '/' && java_parse_traditional_comment(parser)) {
      continue;
    }
    if (ch == '/' && java_parse_eol_comment(parser)) {
      return;
    }
    break;
  }
  java_parse_eol(parser);
}
