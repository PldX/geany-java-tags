#ifndef STRING_TEXT_H_
#define STRING_TEXT_H_

#include "text.h"

// Creates a Text interface for an in-memory string text with initial content.
// Returned value can be manipulated through text* methods and should be free with text_free.
Text* string_text_new(const char* initial);

// Gets the entire text content.
// Returned value must be freed with gfree.
char* string_text_get(Text* text);

#endif // STRING_TEXT_H_
