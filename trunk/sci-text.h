#ifndef SCI_TEXT_H_
#define SCI_TEXT_H_

#include "text.h"

#include "geanyplugin.h"  // ScintillaObject

// Creates a Text interface for a ScintillaObject document.
// Returned value can be manipulated through text* methods and should be free with text_free.
Text* sci_text_new(ScintillaObject* sci);

#endif // SCI_TEXT_H_
