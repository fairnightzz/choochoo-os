#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct String
{
  char string[100];
} String;

// Standard string functions (wrapping helps char* to not destructure)
String make_string(char *charString);
int str_len(String *str);
void string_concat(String *dest, String *src);
void string_add_char(String *dest, char ch);
void string_pop_char(String *dest);
void string_remove(String *dest);
bool string_equal(String *str1, String *str2);

// String formatting is useful for outputting to our console buffer
String string_format(char *fmt, ...);

#endif