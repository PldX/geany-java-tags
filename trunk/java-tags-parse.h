#ifndef JAVA_TAGS_PARSE_H_
#define JAVA_TAGS_PARSE_H_

#include <glib.h>

#include "java-tags-store.h"

// Parses java tags in the path recusively and store them in the tags_store.
void java_tags_parse(const gchar* path, JavaTagsStore* tags_store);

// Asynchronous parser for java tags.
typedef struct _JavaTagsParser {
  // Paths to scan and parse. Owned.
  GPtrArray* paths;
  // Store for parsed tags. Not owned unless orphaned.
  JavaTagsStore* tags_store;
  // private: Mutex.
  GMutex* _mutex;
  // private: Worker thread.
  GThread* _thread;
  // private: Abort signal.
  gboolean _abort;
  // private: Orphan flag.
  gboolean _orphan;
} JavaTagsParser;

// Constructor.
// @param paths The paths to parse. Takes ownership.
// @param store Store for parsed tags. Must be available for the parser lifetime.
JavaTagsParser* jt_parser_new(GPtrArray* paths, JavaTagsStore* store);
// Destructor: aborts and waits for completion before destroying data.
void jt_parser_free(JavaTagsParser* parser);

// Starts the effective scanning and parsing. Can be called a single time for a parser.
void jt_parser_start(JavaTagsParser* parser);
// Waits until the parser has finished.
// Thread safe.
void jt_parser_wait(JavaTagsParser* parser);

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
