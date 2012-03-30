// Defines a minimal abstract interface for interacting with a text document.
// There are concrete implementation for ScintillaObject, file and string.

#ifndef TEXT_H_
#define TEXT_H_

typedef char (*VirtualTextGetChar)(void*,unsigned int);
typedef char* (*VirtualTextGetRange)(void*,unsigned int,int);
typedef void (*VirtualTextSetRange)(void*,unsigned int,int,const char*);
typedef int (*VirtualTextIsEof)(void*,unsigned int);
typedef void (*VirtualTextFree)(void*);

// Encapsulates implementation specific data for a text document.
// Do not use directly but only through text_* methods.
typedef struct _Text {
  void* data;
  VirtualTextGetChar get_char_impl;
  VirtualTextGetRange get_range_impl;
  VirtualTextSetRange set_range_impl;
  VirtualTextIsEof is_eof_impl;
  VirtualTextFree free_impl;
} Text;

// Definition of a text position.
// NOTE: A position is not an anchor (i.e. auto-update after inserts/deletes) for several reasons:
// * Underlying text can be modified outside the text interface (e.g. user input for ScintillaObject).
// * Anchors doesn't guarantee that a parsed structure is still correct (e.g. insert '{' in a java text).
typedef int TextPosition;

// Creates a new text document with the specified implementation.
// To be used only by concrete implementations.
Text* text_new(void* data, 
               VirtualTextGetChar get_char_impl,
               VirtualTextGetRange get_range_impl,
               VirtualTextSetRange set_range_impl,
               VirtualTextIsEof is_eof_impl,
               VirtualTextFree free_impl);

// Reads a character at a random position in text.
// Returns '\0' if position is after eof.
char text_get_char(Text* text, unsigned int pos);

// Reads a null-terminated string of a given length from specified position.
// Returned string must freed using gfree.
// A NULL string is returned when the postion is after eof.
// A negative length indicates that the entire string starting at pos should be read.
// Stops at eof if necessary.
char* text_get_range(Text* text, unsigned int pos, int length);

// Replaces a range with another string.
// A position >= text length means append.
// A negative length means the entire string remainder should be replaced.
void text_set_range(Text* text, unsigned int pos, int length, const char* fragment);

// Test if a given position is past eof.
int text_is_eof(Text* text, unsigned int pos);

// Frees a text document.
void text_free(Text* text);

#endif // TEXT_H_
