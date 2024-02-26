#include "string.h"
#include "util.h"

string new_string(void)
{
  string string = {
      .data = {0},
      .length = 0,
  };
  return string;
}

void string_copy(char *dest, char *src)
{
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
}

string to_string(const char *cstring)
{
  string new_str = new_string();

  int idx = 0;

  while (cstring[idx] != '\0')
  {
    push_char(&new_str, cstring[idx]);
    idx += 1;
  }

  return new_str;
}

int push_char(string *str, char c)
{
  if (str->length >= MAX_STRING_LEN)
    return -1;

  str->data[str->length] = c;
  str->length += 1;
  str->data[str->length] = '\0';

  return 0;
}

void pop_char(string *str)
{
  if (str->length == 0)
  {
    return;
  }

  str->length -= 1;
  str->data[str->length] = '\0';
}

void string_concat(string *destination, string *source)
{
  for (int i = 0; i < str_length(source); i++)
  {
    push_char(destination, source->data[i]);
  }
}

const char *get_data(string *str)
{
  return (const char *)str->data;
}

int str_length(string *str)
{
  return str->length;
}

void str_clear(string *str)
{
  str->length = 0;
  str->data[0] = '\0';
}

int str_cmp(string *s1, string *s2)
{
  if (str_length(s1) != str_length(s2))
  {
    return 0;
  }

  for (int i = 0; i < str_length(s1); i++)
  {
    if (s1->data[i] != s2->data[i])
      return 0;
  }

  return 1;
}

string str_copy(string *s)
{
  string new_str = new_string();
  for (int i = 0; i < str_length(s); i++)
  {
    push_char(&new_str, s->data[i]);
  }
  return new_str;
}

string get_suffix(string *s, int length)
{
  if (str_length(s) <= length)
  {
    return str_copy(s);
  }
  else
  {
    string new_str = new_string();
    for (int i = str_length(s) - length; i < str_length(s); i++)
    {
      push_char(&new_str, s->data[i]);
    }
    return new_str;
  }
}

void char_copy(char *dest, char *src)
{
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
}

// f-style string formatting, with limited format support
string _string_format(char *fmt, va_list va)
{
  string dest = new_string();
  char ch;

  for (;;)
  {
    ch = *(fmt++);
    char bf[12];
    if (ch == '\0')
    {
      break;
    }
    if (ch != '%')
    {
      push_char(&dest, ch);
    }
    else
    {
      string temp = new_string();
      ch = *(fmt++);
      switch (ch)
      {
      case 'u':
        ui2a(va_arg(va, unsigned int), 10, bf);
        temp = to_string(bf);
        string_concat(&dest, &temp);
        break;
      case 'd':
        i2a(va_arg(va, int), bf);
        temp = to_string(bf);
        string_concat(&dest, &temp);
        break;
      case 'x':
        ui2a(va_arg(va, unsigned int), 16, bf);
        temp = to_string(bf);
        string_concat(&dest, &temp);
        break;
      case 's':
        temp = to_string(va_arg(va, char *));
        string_concat(&dest, &temp);
        break;
      case '%':
        push_char(&dest, ch);
        break;
      case '\0':
        break;
      }
    }
  }

  return dest;
}

string string_format(char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  string formattedString = _string_format(fmt, va);
  va_end(va);

  return formattedString;
}
