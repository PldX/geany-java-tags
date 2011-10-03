#include "java-tags-store.h"

static JavaTag* java_tag_new(const gchar* name, const gchar* fullname);
static void java_tag_free(JavaTag* tag);
static gint java_tags_compare(const JavaTag* l, const JavaTag* r, const gchar* name);

JavaTagsStore* java_tags_store_new() {
  JavaTagsStore* store = g_new(JavaTagsStore, 1);
  store->sequence = g_sequence_new((GDestroyNotify) java_tag_free);
  return store;
}

void java_tags_store_free(JavaTagsStore* store) {
  if (store) {
    g_sequence_free(store->sequence);
    g_free(store);
  }
}

void java_tags_store_add(JavaTagsStore* store, gchar* tag_name, gchar* tag_fullname) {
  JavaTag* tag = java_tag_new(tag_name, tag_fullname);
  g_sequence_insert_sorted(store->sequence, tag, (GCompareDataFunc) java_tags_compare, NULL);
}

JavaTagsStoreIter* java_tags_store_search(JavaTagsStore* store, gchar* tag_name) {
  JavaTagsStoreIter* iter = g_new(JavaTagsStoreIter, 1);
  iter->store = store;
  iter->iter = g_sequence_search(store->sequence, NULL, (GCompareDataFunc) java_tags_compare, tag_name);
  return iter;
}

JavaTag* java_tags_store_iter_get(JavaTagsStoreIter* iter) {
  return (JavaTag*) g_sequence_get(iter->iter);
}

void java_tags_store_iter_next(JavaTagsStoreIter* iter) {
  iter->iter = g_sequence_iter_next (iter->iter);
}

gboolean java_tags_store_iter_done(JavaTagsStoreIter* iter) {
  return g_sequence_iter_is_end(iter->iter);
}

void java_tags_store_iter_free(JavaTagsStoreIter* iter) {
  // g_free(iter->iter);
  g_free(iter);
}

static JavaTag* java_tag_new(const gchar* name, const gchar* fullname) {
  JavaTag* tag = g_new(JavaTag, 1);
  tag->name = g_strdup(name);
  tag->fullname = g_strdup(fullname);
}

static void java_tag_free(JavaTag* tag) {
  g_free(tag->name);
  g_free(tag->fullname);
  g_free(tag);
}

static gint java_tags_compare(const JavaTag* l, const JavaTag* r, const gchar* name) {
  // Compare against name first.
  if (name) {
    JavaTag fake_tag;
    fake_tag.name = (gchar*) name;
    fake_tag.fullname = "";
    if (l) {
      return java_tags_compare(l, &fake_tag, NULL);
    } else {
      return java_tags_compare(&fake_tag, r, NULL);
    }
  }
  // Compare the entries.
  gint result = g_strcmp0(l->name, r->name);
  if (!result) {
    result = g_strcmp0(l->fullname, r->fullname);
  }
  return result;
}
