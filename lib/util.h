#ifndef _util_h_
#define _util_h_ 1

#include <stddef.h>
#include <stdint.h>

#define countof(a)   (sizeof(a) / sizeof(*(a)))

// conversions
int a2d(char ch);
char a2i(char ch, char **src, int base, int *nump);
void ui2a(unsigned int num, unsigned int base, char *bf);
void i2a(int num, char *bf);

// memory
void *memset(void *s, int c, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);

// miscellanious
void format_clock(unsigned int time, char *buffer);

int min(int a, int b);

#endif /* util.h */
