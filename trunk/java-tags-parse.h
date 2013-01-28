#ifndef JAVA_TAGS_PARSE_H_
#define JAVA_TAGS_PARSE_H_

#include <glib.h>

#include "java-tags-store.h"

// Parses java tags in the path recusively and store them in the tags_store.
void java_tags_parse(const gchar* path, JavaTagsStore* tags_store);

// Parser stats.
typedef struct _JavaTagsParserStats {
  // Scanned files and directories.
  gint nodes;
  // Scanned directories.
  gint dirs;
  // Parsed files.
  gint files;
  // Tags.
  gint tags;
} JavaTagsParserStats;

// Asynchronous parser for java tags.
typedef struct _JavaTagsParser {
  // Paths to scan and parse. Owned.
  GPtrArray* paths;
  // Store for parsed tags. Not owned unless orphaned.
  JavaTagsStore* tags_store;
  // private: Lock for tags store.
  GStaticRWLock _tags_store_lock;
  // private: Indicates whether it owns the tags store.
  gboolean _owns_store;
  // private: Mutex.
  GMutex* _mutex;
  // private: Worker thread.
  GThread* _thread;
  // private: Stats.
  JavaTagsParserStats _stats; // Updated only by the worker thread.
  // private: Thread safe stats.
  JavaTagsParserStats _safe_stats;
  // private: Abort signal.
  gboolean _abort;
  // private: Reference counting.
  gint _ref_count;
} JavaTagsParser;

// Constructor: creates a parser with reference count set to 1.
// @param paths The paths to parse. Takes ownership.
// @param store Store for parsed tags. Must be available for the parser lifetime.
JavaTagsParser* jt_parser_new(GPtrArray* paths, JavaTagsStore* store);
// Increments the reference count. Should have a paired jt_parser_release when no longer needed.
void jt_parser_add_ref(JavaTagsParser* parser);
// Notifies parser as being less used... automatically destroys the parser and
// its content when no longer referenced.
void jt_parser_release(JavaTagsParser* parser);

// Starts the effective scanning and parsing. Can be called a single time for a parser.
void jt_parser_start(JavaTagsParser* parser);
// Indicates whether the parser is actively performing any work.
gboolean jt_parser_working(JavaTagsParser* parser);
// Suspend updates to store for reading purposes.
void jt_parser_suspend(JavaTagsParser* parser);
// Resumes updates to store.
void jt_parser_resume(JavaTagsParser* parser);
// Waits until the parser has finished.
// Thread safe.
void jt_parser_wait(JavaTagsParser* parser);

// Gets stats from parser.
void jt_parser_get_stats(JavaTagsParser* parser, JavaTagsParserStats* stats);

// Signals the parser to abort as soon as possible. Has no effect if already finished or aborted.
// Waiting may return quicker.
// Thread safe.
void jt_parser_abort(JavaTagsParser* parser);
// Thread safe query for abort state.
gboolean jt_parser_aborted(JavaTagsParser* parser);

// Instructs the parser to auto delete its resources after completion. The deletion
// of the parser may occur at any time during or after method invocation.
// It is recommended to abort the parser before orphaning.
// Not thread safe: any operation concurring with orphaning has undefined behavior.
void jt_parser_orphan(JavaTagsParser* parser);

#endif // JAVA_TAGS_PARSE_H_
