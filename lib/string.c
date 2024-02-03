#include <stdarg.h>
#include "string.h"

void string_copy(char *dest, char *src)
{
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
}

String make_string(char *charString)
{
  String str = {.string = ""};
  string_copy(str.string, charString);
  return str;
}

int str_len(String *str)
{
  char *string = str->string;
  int length = 0;
  while (string[length] != '\0')
  {
    length++;
  }
  return length;
}

void string_concat(String *destination, String *source)
{
  char *dest = destination->string;
  char *src = source->string;
  while (*dest)
  {
    dest++;
  }

  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
}

void string_add_char(String *destination, char ch)
{
  char *dest = destination->string;
  while (*dest)
  {
    dest++;
  }

  *dest++ = ch;
  *dest = '\0';
}

void string_pop_char(String *destination)
{
  char *dest = destination->string;
  while (*dest)
  {
    dest++;
  }

  dest--;
  *dest = '\0';
}

void string_remove(String *destination)
{
  char *dest = destination->string;
  *dest = '\0';
}

bool string_equal(String *string1, String *string2)
{
  char *str1 = string1->string;
  char *str2 = string2->string;

  while (*str1 && (*str1 == *str2))
  {
    str1++;
    str2++;
  }

  return *str1 == *str2;
}

char *string_puts(char *cur, char *buf)
{
  while (*buf)
  {
    *cur = *buf;
    cur++;
    buf++;
  }
  return cur;
}
