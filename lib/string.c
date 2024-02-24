#include "string.h"

string new_string(void)
{
  string string = {
      .data = {0},
      .length = 0,
  };
  return string;
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
