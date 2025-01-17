#ifndef _rpi_h_
#define _rpi_h_ 1

#include <stdint.h>
#include <stddef.h>

// Serial line 1 on the RPi hat is used for the console; 2 for Marklin
#define CONSOLE 1
#define MARKLIN 2

// Timer
uint64_t timer_get();

void gpio_init();
void uart_config_and_enable(size_t line);
unsigned char uart_getc(size_t line);
// Non blocking get c
int uart_getcnow(size_t line, unsigned char *data);
void uart_putc(size_t line, unsigned char c);
// None blocking putc
int uart_try_putc(size_t line, unsigned char c);
void uart_putl(size_t line, const char *buf, size_t blen);
void uart_puts(size_t line, const char *buf);
void uart_printf(size_t line, char *fmt, ...);

uint32_t get_time();
void delay(uint32_t amount);

#endif /* rpi.h */
