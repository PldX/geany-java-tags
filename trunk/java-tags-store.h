#ifndef JAVA_TAGS_STORE_H_
#define JAVA_TAGS_STORE_H_

#include <glib.h>

typedef struct _JavaTag {
  gchar* name;
  gchar* fullname;
} JavaTag;
typedef struct _JavaTagsStore {
  GSequence* sequence;
} JavaTagsStore;
typedef struct _JavaTagsStoreIter {
  JavaTagsStore* store;
  GSequenceIter* iter;
} JavaTagsStoreIter;

// Constructor.
JavaTagsStore* java_tags_store_new();
// Destructor.
void java_tags_store_free(JavaTagsStore* store);

// Inserts a new tag in the store.
void java_tags_store_add(JavaTagsStore* store, gchar* tag_name, gchar* tag_fullname);

// Search first element higher or equal by tag name.
JavaTagsStoreIter* java_tags_store_search(JavaTagsStore* store, gchar* tag_name);

// Get element by iterator.
JavaTag* java_tags_store_iter_get(JavaTagsStoreIter* iter);
// Advances iterator.
void java_tags_store_iter_next(JavaTagsStoreIter* iter);
// Tests whether iterator is still valid.
gboolean java_tags_store_iter_done(JavaTagsStoreIter* iter);
// Releases the iterator.
void java_tags_store_iter_free(JavaTagsStoreIter* iter);

#endif // JAVA_TAGS_STORE_H_
