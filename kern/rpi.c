#include "rpi.h"
#include "util.h"
#include <stdarg.h>
#include <stdbool.h>
#include "stdlib.h"

static char *const MMIO_BASE = (char *)0xFE000000;

/*********** GPIO CONFIGURATION ********************************/

static char *const GPIO_BASE = (char *)(MMIO_BASE + 0x200000);
static const uint32_t GPFSEL_OFFSETS[6] = {0x00, 0x04, 0x08, 0x0c, 0x10, 0x14};
static const uint32_t GPIO_PUP_PDN_CNTRL_OFFSETS[4] = {0xe4, 0xe8, 0xec, 0xf0};
static BQueue CHAR_OUT_Q_MARKLIN;
static BQueue CHAR_OUT_Q_CONSOLE;

#define GPFSEL_REG(reg) (*(uint32_t *)(GPIO_BASE + GPFSEL_OFFSETS[reg]))
#define GPIO_PUP_PDN_CNTRL_REG(reg) (*(uint32_t *)(GPIO_BASE + GPIO_PUP_PDN_CNTRL_OFFSETS[reg]))

// function control settings for GPIO pins
static const uint32_t GPIO_INPUT = 0x00;
static const uint32_t GPIO_OUTPUT = 0x01;
static const uint32_t GPIO_ALTFN0 = 0x04;
static const uint32_t GPIO_ALTFN1 = 0x05;
static const uint32_t GPIO_ALTFN2 = 0x06;
static const uint32_t GPIO_ALTFN3 = 0x07;
static const uint32_t GPIO_ALTFN4 = 0x03;
static const uint32_t GPIO_ALTFN5 = 0x02;

// pup/pdn resistor settings for GPIO pins
static const uint32_t GPIO_NONE = 0x00;
static const uint32_t GPIO_PUP = 0x01;
static const uint32_t GPIO_PDP = 0x02;

static void setup_gpio(uint32_t pin, uint32_t setting, uint32_t resistor)
{
  uint32_t reg = pin / 10;
  uint32_t shift = (pin % 10) * 3;
  uint32_t status = GPFSEL_REG(reg); // read status
  status &= ~(7u << shift);          // clear bits
  status |= (setting << shift);      // set bits
  GPFSEL_REG(reg) = status;

  reg = pin / 16;
  shift = (pin % 16) * 2;
  status = GPIO_PUP_PDN_CNTRL_REG(reg); // read status
  status &= ~(3u << shift);             // clear bits
  status |= (resistor << shift);        // set bits
  GPIO_PUP_PDN_CNTRL_REG(reg) = status; // write back
}

/*********** UART CONTROL ************************ ************/

static char *const UART0_BASE = (char *)(MMIO_BASE + 0x201000);
static char *const UART3_BASE = (char *)(MMIO_BASE + 0x201600);

// line_uarts[] maps the each serial line on the RPi hat to the UART that drives it
// currently:
//   * there is no line 0
//   * line 1 (console) is driven by RPi UART0
//   * line 2 (train control) is driven by RPi UART3
static char *const line_uarts[] = {NULL, UART0_BASE, UART3_BASE};

// UART register offsets
static const uint32_t UART_DR = 0x00;
static const uint32_t UART_FR = 0x18;
static const uint32_t UART_IBRD = 0x24;
static const uint32_t UART_FBRD = 0x28;
static const uint32_t UART_LCRH = 0x2c;
static const uint32_t UART_CR = 0x30;

#define UART_REG(line, offset) (*(volatile uint32_t *)(line_uarts[line] + offset))

// masks for specific fields in the UART registers
static const uint32_t UART_FR_RXFE = 0x10;
static const uint32_t UART_FR_TXFF = 0x20;
static const uint32_t UART_FR_RXFF = 0x40;
static const uint32_t UART_FR_TXFE = 0x80;

static const uint32_t UART_CR_UARTEN = 0x01;
static const uint32_t UART_CR_LBE = 0x80;
static const uint32_t UART_CR_TXE = 0x100;
static const uint32_t UART_CR_RXE = 0x200;
static const uint32_t UART_CR_RTS = 0x800;
static const uint32_t UART_CR_RTSEN = 0x4000;
static const uint32_t UART_CR_CTSEN = 0x8000;

static const uint32_t UART_LCRH_PEN = 0x2;
static const uint32_t UART_LCRH_EPS = 0x4;
static const uint32_t UART_LCRH_STP2 = 0x8;
static const uint32_t UART_LCRH_FEN = 0x10;
static const uint32_t UART_LCRH_WLEN_LOW = 0x20;
static const uint32_t UART_LCRH_WLEN_HIGH = 0x40;
static const unsigned char UART_BLOCKING = 0xF0;

// GPIO initialization, to be called before UART functions.
// GPIO pins 14 & 15 already configured by boot loader, but redo for clarity.
// For UART3 (line 2 on the RPi hat), we need to configure the GPIO to route
// the uart control and data signals to the GPIO pins expected by the hat.
void gpio_init()
{
  setup_gpio(4, GPIO_ALTFN4, GPIO_NONE);
  setup_gpio(5, GPIO_ALTFN4, GPIO_NONE);
  setup_gpio(6, GPIO_ALTFN4, GPIO_NONE);
  setup_gpio(7, GPIO_ALTFN4, GPIO_NONE);
  setup_gpio(14, GPIO_ALTFN0, GPIO_NONE);
  setup_gpio(15, GPIO_ALTFN0, GPIO_NONE);
}

// Configure the line properties (e.g, parity, baud rate) of a UART and ensure that it is enabled
void uart_config_and_enable(size_t line)
{

  uint32_t baud_ival, baud_fval;
  CHAR_OUT_Q_CONSOLE = new_byte_queue();
  CHAR_OUT_Q_MARKLIN = new_byte_queue();
  switch (line)
  {
  // setting baudrate to approx. 115246.09844 (best we can do)
  case CONSOLE:
    baud_ival = 26;
    baud_fval = 2;
    break;
  // setting baudrate to 2400
  case MARKLIN:
    baud_ival = 1258;
    baud_fval = 0;
    break;
  default:
    return;
  }

  // line control registers should not be changed while the UART is enabled, so disable it
  uint32_t cr_state = UART_REG(line, UART_CR);
  UART_REG(line, UART_CR) = cr_state & ~UART_CR_UARTEN;

  // set the baud rate
  UART_REG(line, UART_IBRD) = baud_ival;
  UART_REG(line, UART_FBRD) = baud_fval;
  switch (line)
  {
  case CONSOLE:
    UART_REG(line, UART_LCRH) = UART_LCRH_WLEN_HIGH | UART_LCRH_WLEN_LOW | UART_LCRH_FEN;
    break;
  case MARKLIN:
    UART_REG(line, UART_LCRH) = UART_LCRH_WLEN_HIGH | UART_LCRH_WLEN_LOW | UART_LCRH_FEN | UART_LCRH_STP2;
    break;
  }

  UART_REG(line, UART_CR) = cr_state | UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

unsigned char uart_getc(size_t line)
{
  unsigned char ch;
  /* wait for data if necessary */
  while (UART_REG(line, UART_FR) & UART_FR_RXFE)
    ;
  ch = UART_REG(line, UART_DR);
  return (ch);
}

bool uart_non_blocking_getc(size_t line, unsigned char *ch)
{
  if (UART_REG(line, UART_FR) & UART_FR_RXFE)
  {
    return false;
  }
  *ch = UART_REG(line, UART_DR);
  return true;
}

unsigned char uart_nb_getc(size_t line)
{
  unsigned char ch;
  if (UART_REG(line, UART_FR) & UART_FR_RXFE)
  {
    return UART_BLOCKING;
  }
  ch = UART_REG(line, UART_DR);
  return (ch);
}

void try_uart_out(void)
{
  while (length(&CHAR_OUT_Q_CONSOLE) > 0)
  {
    if (UART_REG(CONSOLE, UART_FR) & UART_FR_TXFF)
    {
      break;
    }
    else
    {
      UART_REG(CONSOLE, UART_DR) = pop(&CHAR_OUT_Q_CONSOLE);
    }
  }

  while (length(&CHAR_OUT_Q_MARKLIN) > 0)
  {
    if (UART_REG(MARKLIN, UART_FR) & UART_FR_TXFF)
    {
      break;
    }
    else
    {
      UART_REG(MARKLIN, UART_DR) = pop(&CHAR_OUT_Q_MARKLIN);
    }
  }
}

void uart_putc(size_t line, unsigned char c)
{
  switch (line)
  {
  case CONSOLE:
    push(&CHAR_OUT_Q_CONSOLE, c);
    break;
  case MARKLIN:
    push(&CHAR_OUT_Q_MARKLIN, c);
    break;
  };
  try_uart_out();
}

void uart_blocking_putc(size_t line, unsigned char c)
{
  // make sure there is room to write more data
  while (UART_REG(line, UART_FR) & UART_FR_TXFF)
    ;
  UART_REG(line, UART_DR) = c;
}

bool uart_non_blocking_putc(size_t line, unsigned char c)
{
  if (UART_REG(line, UART_FR) & UART_FR_TXFF)
    return false;
  UART_REG(line, UART_DR) = c;
  return true;
}

void uart_putl(size_t line, const char *buf, size_t blen)
{
  size_t i;
  for (i = 0; i < blen; i++)
  {
    uart_putc(line, *(buf + i));
  }
}

void uart_puts(size_t line, const char *buf)
{
  while (*buf)
  {
    uart_putc(line, *buf);
    buf++;
  }
}

void uart_blocking_puts(size_t line, const char *buf)
{
  while (*buf)
  {
    uart_blocking_putc(line, *buf);
    buf++;
  }
}

// printf-style printing, with limited format support
void uart_format_print(size_t line, char *fmt, va_list va)
{
  char bf[12];
  char ch;

  while ((ch = *(fmt++)))
  {
    if (ch != '%')
      uart_putc(line, ch);
    else
    {
      ch = *(fmt++);
      switch (ch)
      {
      case 'u':
        ui2a(va_arg(va, unsigned int), 10, bf);
        uart_puts(line, bf);
        break;
      case 'd':
        i2a(va_arg(va, int), bf);
        uart_puts(line, bf);
        break;
      case 'x':
        ui2a(va_arg(va, unsigned int), 16, bf);
        uart_puts(line, bf);
        break;
      case 's':
        uart_puts(line, va_arg(va, char *));
        break;
      case '%':
        uart_putc(line, ch);
        break;
      case '\0':
        return;
      }
    }
  }
}

static void uart_format_blocking_print(size_t line, char *fmt, va_list va)
{
  char bf[12];
  char ch;

  while ((ch = *(fmt++)))
  {
    if (ch != '%')
      uart_blocking_putc(line, ch);
    else
    {
      ch = *(fmt++);
      switch (ch)
      {
      case 'u':
        ui2a(va_arg(va, unsigned int), 10, bf);
        uart_blocking_puts(line, bf);
        break;
      case 'd':
        i2a(va_arg(va, int), bf);
        uart_blocking_puts(line, bf);
        break;
      case 'x':
        ui2a(va_arg(va, unsigned int), 16, bf);
        uart_blocking_puts(line, bf);
        break;
      case 's':
        uart_blocking_puts(line, va_arg(va, char *));
        break;
      case '%':
        uart_blocking_putc(line, ch);
        break;
      case '\0':
        return;
      }
    }
  }
}

void uart_printf(size_t line, char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  uart_format_print(line, fmt, va);
  va_end(va);
}

void uart_blocking_printf(size_t line, char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  uart_format_blocking_print(line, fmt, va);
  va_end(va);
}

/*********** SYSTEM TIMER CONFIGURATION ********************************/

static char *const SYS_TIME_BASE = (char *)(MMIO_BASE + 0x00003000);
// UART register offsets
static const uint32_t SYS_TIME_CS = 0x00;
static const uint32_t SYS_TIME_CLO = 0x04;
static const uint32_t SYS_TIME_CHI = 0x08;
static const uint32_t SYS_TIME_C0 = 0x0c;
static const uint32_t SYS_TIME_C1 = 0x10;
static const uint32_t SYS_TIME_C2 = 0x14;
static const uint32_t SYS_TIME_C3 = 0x18;

#define SYS_TIME_REG(offset) (*(volatile uint32_t *)(SYS_TIME_BASE + offset))

uint64_t get_systime()
{
  uint64_t time;
  time = SYS_TIME_REG(SYS_TIME_CLO);
  time |= (uint64_t)SYS_TIME_REG(SYS_TIME_CHI) << 32;
  return time;
}