#include "java-parser.h"

#include <glib.h>

#include "string-text.h"
#include "test.h"
#include "text.h"
#include "utils.h"

JavaDocStructure* parse_text(const char* java_text, 
                             gboolean extract_package, 
                             gboolean extract_imports) {
  JavaParseOptions opts;
  java_parse_opts_init(&opts);
  opts.extract_package = extract_package;
  opts.extract_imports = extract_imports;
  Text* text = string_text_new(java_text);
  JavaDocStructure* java_doc = java_parse(text, &opts);
  text_free(text);
  return java_doc;
}

void assert_parse_package(const char* package_name, const char* java_text) {
  JavaDocStructure* java_doc = parse_text(java_text, TRUE, FALSE);
  ASSERT_NOT_NULL(java_doc);
  ASSERT_NOT_NULL(java_doc->package_declaration);
  ASSERT_STR_EQ(package_name, java_doc->package_declaration->package_name);
  java_doc_free(java_doc);
}

void test_parse_package() {
  assert_parse_package("geany.java.test",
      "package geany.java.test;");
  assert_parse_package("test",
    "//Comment\n"
    "package test;\n"
    "");
  assert_parse_package("test",
    "//Comment\n"
    "package test; // Post comment\n"
    "");
  assert_parse_package("test",
    "/* Multi line comment */\n"
    "package test; // Post comment");
  assert_parse_package("p.i.c",
    "\n"
    "package p.i.c;\n"
    "\n"
    "import java.util.Test;\n"
    "\n"
    "public class Test {}");
}

void test_parse_imports() {
  JavaDocStructure* java_doc = parse_text(
      "import java.doc;\r\n"
      "import test.x; // Post comment\n"
      "// Comment \n"
      "import test.y; /* Post comment*/\n"
      "import /*static*/ test.x;\r"
      "import static test.x.SA;\n"
      "\n",
      FALSE, TRUE);
  ASSERT_NOT_NULL(java_doc);
  ASSERT_NOT_NULL(java_doc->import_declarations);
  int imports = ARRAY_LEN(java_doc->import_declarations);
  ASSERT_EQ(5, imports);
  java_doc_free(java_doc);
}

int main(int argc, char** argv) {
  RUN_TEST(test_parse_package);
  RUN_TEST(test_parse_imports);
  return 0;
}
