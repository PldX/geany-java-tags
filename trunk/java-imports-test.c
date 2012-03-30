#include "java-imports.h"

#include "string-text.h"
#include "test.h"

static const char* IMPORT_GROUPS_ORDER[] = {
  "static:",
  "org.geany",
  "org.geany.java",
  "java",
  "javax",
};

void assert_insert_import(const char* initial_text,
                          const char* import,
                          const char* expected_text) {
  Text* text = string_text_new(initial_text);
  java_import(import, IMPORT_GROUPS_ORDER, text);
  char* actual_text = string_text_get(text);
  ASSERT_STR_EQ(expected_text, actual_text);
  g_free(actual_text);
  text_free(text);
}

void test_empty_file() {
  assert_insert_import(
      "",
      "java.util.List",
      "import java.util.List;\n"
      "\n");
}

void test_insert_first_import() {
  assert_insert_import(
      "class",
      "java.util.List",
      "import java.util.List;\n"
      "\n"
      "class");
}

void test_insert_after_package() {
  assert_insert_import(
      "// Some comment\n"
      "package geany;\n"
      "\n"
      "class Test {}",
      "java.util.List",
      "// Some comment\n"
      "package geany;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class Test {}");
}

void test_insert_first_in_group() {
  assert_insert_import(
      "import static Object.equals;\n"
      "\n"
      "import java.util.Map;\n"
      "\n",
      "java.util.List",
      "import static Object.equals;\n"
      "\n"
      "import java.util.List;\n"
      "import java.util.Map;\n"
      "\n");
}

void test_insert_in_middle_of_group() {
  assert_insert_import(
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.Test;\n"
      "\n",
      "org.geany.String",
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.String;\n"
      "import org.geany.Test;\n"
      "\n");
}

void test_insert_last_in_group() {
  assert_insert_import(
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.String;\n"
      "\n"
      "import java.util.List;\n"
      "\n",
      "org.geany.Test",
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.String;\n"
      "import org.geany.Test;\n"
      "\n"
      "import java.util.List;\n"
      "\n");
}

void test_insert_first_new_group() {
  assert_insert_import(
      "package geany;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class",
      "org.geany.Plugin",
      "package geany;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class");
}

void test_insert_middle_new_group() {
  assert_insert_import(
      "import static Object.equals;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class",
      "org.geany.Plugin",
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class");
}

void test_insert_last_new_group() {
  assert_insert_import(
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.Test;\n"
      "\n"
      "class",
      "java.util.List",
      "import static Object.equals;\n"
      "\n"
      "import org.geany.Plugin;\n"
      "import org.geany.Test;\n"
      "\n"
      "import java.util.List;\n"
      "\n"
      "class");
}

int main(int argc, char** argv) {
  RUN_TEST(test_empty_file);
  RUN_TEST(test_insert_first_import);
  RUN_TEST(test_insert_after_package);
  RUN_TEST(test_insert_first_in_group);
  RUN_TEST(test_insert_in_middle_of_group);
  RUN_TEST(test_insert_last_in_group);
  RUN_TEST(test_insert_first_new_group);
  RUN_TEST(test_insert_middle_new_group);
  RUN_TEST(test_insert_last_new_group);
  return 0;
}
