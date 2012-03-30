#include "string-text.h"

#include "test.h"

void test_set_range() {
  Text* text = string_text_new("");
  text_set_range(text, 0, 0, "test");
  char* all = text_get_range(text, 0, 4);
  ASSERT_STR_EQ("test", all);
  g_free(all);
  text_set_range(text, 2, 1, "mpes");
  all = text_get_range(text, 0, 7);
  ASSERT_STR_EQ("tempest", all);
  g_free(all);
  text_set_range(text, 3, 1, "");
  all = text_get_range(text, 0, 7);
  ASSERT_STR_EQ("temest", all);
  g_free(all);
  text_set_range(text, 100, 1, "ed");
  all = text_get_range(text, 0, -1);
  ASSERT_STR_EQ("temested", all);
  g_free(all);
  text_set_range(text, 3, 0, "-");
  all = text_get_range(text, 0, -1);
  ASSERT_STR_EQ("tem-ested", all);
  g_free(all);
  text_free(text);
}

void test_get_char() {
  Text* text = string_text_new("");
  text_set_range(text, 0, 0, "test");
  ASSERT_EQ(0, text_get_char(text, -1));
  ASSERT_EQ('t', text_get_char(text, 0));
  ASSERT_EQ('e', text_get_char(text, 1));
  ASSERT_EQ('s', text_get_char(text, 2));
  ASSERT_EQ('t', text_get_char(text, 3));
  ASSERT_EQ(0, text_get_char(text, 4));
  text_free(text);
}

void test_get_range() {
  Text* text = string_text_new("");
  text_set_range(text, 0, 0, "test");
  char* range = text_get_range(text, 1, 2);
  ASSERT_STR_EQ("es", range);
  g_free(range);
  range = text_get_range(text, 0, 4);
  ASSERT_STR_EQ("test", range);
  g_free(range);
  range = text_get_range(text, 3, 8);
  ASSERT_STR_EQ("t", range);
  g_free(range);
  range = text_get_range(text, 4, 1);
  ASSERT_STR_EQ("", range);
  g_free(range);
  range = text_get_range(text, 0, 0);
  ASSERT_STR_EQ("", range);
  g_free(range);
  text_free(text);
}

int main(int argc, char** argv) {
  RUN_TEST(test_set_range);
  RUN_TEST(test_get_char);
  RUN_TEST(test_get_range);
  return 0;
}
