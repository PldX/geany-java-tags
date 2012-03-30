// Various utils for scintilla.

#ifndef SCI_UTILS_H_
#define SCI_UTILS_H_

#include "geanyplugin.h"  // ScintillaObject

int is_word_char(gchar ch);

void sci_get_current_word(ScintillaObject* sci, gint* start, gint* end);
gchar* sci_get_current_word_text(ScintillaObject* sci);

void sci_replace_text(ScintillaObject* sci, gint start, gint end, const gchar* text);
void sci_replace_current_word(ScintillaObject* sci, gchar* text);


#endif // SCI_UTILS_H_
