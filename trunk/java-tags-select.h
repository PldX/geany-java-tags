#ifndef JAVA_TAGS_SELECT_H_
#define JAVA_TAGS_SELECT_H_

#include <glib.h>

#include "java-tags-store.h"

/**
 * Shows tag selection dialog.
 * 
 * @param initial_tag Initial tag for prefix filtering.
 * @return User selected tag or {@code null} if there is no selection.
 */
gchar* java_tags_select(JavaTagsStore* tags_store, gchar* initial_tag);

#endif // JAVA_TAGS_SELECT_H_
