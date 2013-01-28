
#include "java-tags-parse.h"

#include <glib.h>

#include "java-tags-store.h"

// Private: frees the parser.
static void jt_parser_free(JavaTagsParser* parser);

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

static void jt_parser_stats_init(JavaTagsParserStats* stats) {
  stats->nodes = 0;
  stats->dirs = 0;
  stats->files = 0;
  stats->tags = 0;
}

static void jt_parser_stats_copy(JavaTagsParserStats* src, JavaTagsParserStats* dst) {
  dst->nodes = src->nodes;
  dst->dirs = src->dirs;
  dst->files = src->files;
  dst->tags = src->tags;
}

static void jt_parser_parse_file(JavaTagsParser* parser, const gchar* dirpath, const gchar* file) {
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
      gchar* tag = g_strcut(file, /*len(".java")*/5);
      gchar* fullname = g_strconcat(package, ".", tag, NULL);
      g_static_rw_lock_writer_lock(&parser->_tags_store_lock);
      java_tags_store_add(parser->tags_store, tag, fullname);
      g_static_rw_lock_writer_unlock(&parser->_tags_store_lock);
      g_free(fullname);
      g_free(tag);
      parser->_stats.tags++;
    }
  }
  g_io_channel_close(channel);
  parser->_stats.files++;
}

static gboolean jt_parser_parse_recursive(JavaTagsParser* parser, const gchar* path) {
  GDir* dir = g_dir_open(path, 0, NULL);

  const gchar* file;
  gboolean aborted = FALSE;
  while (!aborted && (file = g_dir_read_name(dir)) != NULL) {
    if (!g_strcmp0(file, ".") || !g_strcmp0(file, "..")) {
      continue;
    }
    if (parser->_stats.nodes++ % 10 == 1) {
      aborted = jt_parser_aborted(parser);
      g_mutex_lock(parser->_mutex);
      jt_parser_stats_copy(&parser->_stats, &parser->_safe_stats);
      g_mutex_unlock(parser->_mutex);
    }
    gchar* dirpath = g_build_filename(path, file, NULL);
    if (g_file_test(dirpath, G_FILE_TEST_IS_SYMLINK)) {
      // Ignore sym-links.
    } else if (g_file_test(dirpath, G_FILE_TEST_IS_DIR)) {
      aborted = jt_parser_parse_recursive(parser, dirpath);
    } else if (g_str_has_suffix(file, ".java") && g_file_test(dirpath, G_FILE_TEST_IS_REGULAR)) {
      jt_parser_parse_file(parser, dirpath, file);
    }
    g_free(dirpath);
  }

  g_dir_close(dir);
  parser->_stats.dirs++;
  return aborted;
}

JavaTagsParser* jt_parser_new(GPtrArray* paths, JavaTagsStore* tags_store) {
  JavaTagsParser* parser = g_new(JavaTagsParser, 1);
  parser->paths = paths;
  parser->tags_store = tags_store;
  g_static_rw_lock_init(&parser->_tags_store_lock);
  parser->_owns_store = FALSE;
  parser->_mutex = g_mutex_new();
  jt_parser_stats_init(&parser->_stats);
  jt_parser_stats_init(&parser->_safe_stats);
  parser->_thread = NULL;
  parser->_abort = FALSE;
  parser->_ref_count = 1;
}

void jt_parser_add_ref(JavaTagsParser* parser) {
  g_mutex_lock(parser->_mutex);
  parser->_ref_count++;
  g_mutex_unlock(parser->_mutex);
}

void jt_parser_release(JavaTagsParser* parser) {
  gboolean has_references = FALSE;
  g_mutex_lock(parser->_mutex);
  parser->_ref_count--;
  has_references = (parser->_ref_count > 0);
  g_mutex_unlock(parser->_mutex);
  if (!has_references) {
    jt_parser_free(parser);
  }
}

void jt_parser_free(JavaTagsParser* parser) {
  g_mutex_free(parser->_mutex);
  g_ptr_array_free(parser->paths, TRUE);
  if (parser->_owns_store) {
    java_tags_store_free(parser->tags_store);
  }
  g_static_rw_lock_free(&parser->_tags_store_lock);
}

gpointer jt_parser_thread(JavaTagsParser* parser) {
  int i;
  for (i = 0; !jt_parser_aborted(parser) && i < parser->paths->len; ++i) {
    msgwin_status_add("Loading java tags from directory %s.", parser->paths->pdata[i]);
    jt_parser_parse_recursive(parser, (const gchar*) parser->paths->pdata[i]);
    msgwin_status_add("Finished loading java tags in directory %s.", parser->paths->pdata[i]);
  }
  gboolean has_references = TRUE;
  g_mutex_lock(parser->_mutex);
  jt_parser_stats_copy(&parser->_stats, &parser->_safe_stats);
  parser->_thread = NULL;
  parser->_ref_count--;
  has_references = (parser->_ref_count > 0);
  g_mutex_unlock(parser->_mutex);
  if (!has_references) {
    jt_parser_free(parser);
  }
  return NULL;
}

void jt_parser_start(JavaTagsParser* parser) {
  g_mutex_lock(parser->_mutex);
  if (!parser->_thread) {
    parser->_ref_count++;
    parser->_thread = g_thread_create((GThreadFunc) jt_parser_thread, parser, TRUE, NULL);
  }
  g_mutex_unlock(parser->_mutex);
}

gboolean jt_parser_working(JavaTagsParser* parser) {
  gboolean working = FALSE;
  g_mutex_lock(parser->_mutex);
  working = (parser->_thread != NULL);
  g_mutex_unlock(parser->_mutex);
  return working;
}

void jt_parser_suspend(JavaTagsParser* parser) {
  g_static_rw_lock_reader_lock(&parser->_tags_store_lock);
}

void jt_parser_resume(JavaTagsParser* parser) {
  g_static_rw_lock_reader_unlock(&parser->_tags_store_lock);
}

void jt_parser_wait(JavaTagsParser* parser) {
  GThread* thread;
  g_mutex_lock(parser->_mutex);
  thread = parser->_thread;
  g_mutex_unlock(parser->_mutex);
  if (thread) {
    g_thread_join(thread);
  }
}

void jt_parser_get_stats(JavaTagsParser* parser, JavaTagsParserStats* stats) {
  g_mutex_lock(parser->_mutex);
  jt_parser_stats_copy(&parser->_safe_stats, stats);
  g_mutex_unlock(parser->_mutex);
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
  g_mutex_lock(parser->_mutex);
  parser->_owns_store = TRUE;
  g_mutex_unlock(parser->_mutex);
  jt_parser_release(parser);
}

