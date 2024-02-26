#ifndef _string_h_
#define _string_h_ 1

#include <stddef.h>
#include <stdarg.h>

#define MAX_STRING_LEN 128

typedef struct string
{
  char data[MAX_STRING_LEN + 1]; // null-terminated
  int length;
} string;

string new_string(void);
string to_string(const char *char_str);
int push_char(string *str, char c);
void pop_char(string *str);
const char *get_data(string *str);
int str_length(string *str);
void str_clear(string *string);
int str_cmp(string *a, string *b);
string str_copy(string *s);
string get_suffix(string *s, int length);
void char_copy(char *dest, char *src);

// String formatting is useful for outputting to our console buffer
string string_format(char *fmt, ...);
string _string_format(char *fmt, va_list va);

#endif /* string.h */