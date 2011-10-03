
#include "java-tags-parse.h"

#include <glib.h>

#include "java-tags-store.h"

static gsize g_strlen(const gchar* s) {
  gsize size;
  for (size = 0; *s; ++s) {
    ++size;
  }
  return size;
}
static gchar* g_strcut(const gchar* s, gsize cut_size) {
  gsize size = g_strlen(s);
  if (cut_size > size) {
    cut_size = size;
  }
  return g_strndup(s, size-cut_size);
}

typedef struct _JavaTagsParseData {
  gchar* path;
  JavaTagsStore* store;
} JavaTagsParseData;

static gint java_tags_parse_recursive(const gchar* path, JavaTagsStore* tags_store);
static gpointer java_tags_parse_thread(JavaTagsParseData* data);

void java_tags_parse(const gchar* path, JavaTagsStore* tags_store) {
  gint tags_count = java_tags_parse_recursive(path, tags_store);
  g_message("Extracted %d java tags from %s", tags_count, path);
}

static gint java_tags_parse_recursive(const gchar* path, JavaTagsStore* tags_store) {
  gint count = 0;
  GDir* dir = g_dir_open(path, 0, NULL);

  const gchar* file;
  while ((file = g_dir_read_name(dir)) != NULL) {
    if (!g_strcmp0(file, ".") || !g_strcmp0(file, "..")) {
      continue;
    }
    gchar* dirpath = g_build_filename(path, file, NULL);
    if (g_file_test(dirpath, G_FILE_TEST_IS_DIR)) {
      count += java_tags_parse_recursive(dirpath, tags_store);
    } else if (g_str_has_suffix(file, ".java") && g_file_test(dirpath, G_FILE_TEST_IS_REGULAR)) {
      GIOChannel* channel = g_io_channel_new_file(dirpath, "r", NULL);
      if (channel) {
        gchar* package = NULL;
        gchar* line;
        while (!package && g_io_channel_read_line(channel, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
          if (g_str_has_prefix(line, "package")) {
            package = g_strstrip(g_strdup(line + /*len("package")*/7));
            gchar* package_end = g_strstr_len(package, -1, ";");
            if (package_end) {
              *package_end = '\0';
            }
          }
          g_free(line);
        }
        if (package) {
          ++count;
          gchar* tag = g_strcut(file, /*len(".java")*/5);
          gchar* fullname = g_strconcat(package, ".", tag, NULL);
          java_tags_store_add(tags_store, tag, fullname);
          g_free(fullname);
          g_free(tag);
        }
      }
      g_io_channel_close(channel);
    }
    g_free(dirpath);
  }

  g_dir_close(dir);
  return count;
}

JavaTagsParser* jt_parser_new(GPtrArray* paths, JavaTagsStore* tags_store) {
  JavaTagsParser* parser = g_new(JavaTagsParser, 1);
  parser->paths = paths;
  parser->tags_store = tags_store;
  parser->_mutex = g_mutex_new();
  parser->_thread = NULL;
  parser->_abort = FALSE;
  parser->_orphan = FALSE;
}

void jt_parser_free(JavaTagsParser* parser) {
  if (!parser) {
    return;
  }
  jt_parser_abort(parser);
  jt_parser_wait(parser); // joins and release parser->_thread.
  g_mutex_free(parser->_mutex);

  g_ptr_array_free(parser->paths, TRUE);
  if (parser->_orphan) {
    java_tags_store_free(parser->tags_store);
  }
}

gpointer jt_parser_thread(JavaTagsParser* parser) {
  int i;
  for (i = 0; !jt_parser_aborted(parser) && i < parser->paths->len; ++i) {
    java_tags_parse((const gchar*) parser->paths->pdata[i], parser->tags_store);
  }
  return NULL;
}

void jt_parser_start(JavaTagsParser* parser) {
  g_mutex_lock(parser->_mutex);
  if (!parser->_thread) {
    parser->_thread = g_thread_create((GThreadFunc) jt_parser_thread, parser, TRUE, NULL);
  }
  g_mutex_unlock(parser->_mutex);
}

void jt_parser_wait(JavaTagsParser* parser) {
  GThread* thread;
  g_mutex_lock(parser->_mutex);
  thread = parser->_thread;
  g_mutex_unlock(parser->_mutex);
  if (thread) {
    g_thread_join(thread);

    g_mutex_lock(parser->_mutex);
    parser->_thread = NULL;
    g_mutex_unlock(parser->_mutex);
  }
}

void jt_parser_abort(JavaTagsParser* parser) {
  g_mutex_lock(parser->_mutex);
  parser->_abort = TRUE;
  g_mutex_unlock(parser->_mutex);
}

gboolean jt_parser_aborted(JavaTagsParser* parser) {
  gboolean abort;
  g_mutex_lock(parser->_mutex);
  abort = parser->_abort;
  g_mutex_unlock(parser->_mutex);
  return abort;
}

void jt_parser_orphan(JavaTagsParser* parser) {
  // Not guarded b/c it is unsafe anyway (parser may be already deleted).
  parser->_orphan = TRUE;
  // Start a thread for waiting and deleting the parser.
  g_thread_create((GThreadFunc) jt_parser_free, parser, FALSE, NULL);
}

