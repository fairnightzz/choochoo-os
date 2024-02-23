#ifndef _rpi_h_
#define _rpi_h_ 1

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

static char *const MMIO_BASE = (char *)0xFE000000;

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
void uart_format_print(size_t line, char *fmt, va_list va);
unsigned char uart_getc_queued(size_t line, bool* is_buffer_empty);
bool uart_is_rx_interrupt(size_t line);
void uart_clear_rx(size_t line);
bool uart_is_cts_interrupt(size_t line);
bool uart_get_cts(size_t line);
void uart_clear_cts(size_t line);

#endif /* rpi.h */
