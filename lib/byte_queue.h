#ifndef __BYTE_QUEUE_H__
#define __BYTE_QUEUE_H__ 1

#define MAX_QUEUE_LENGTH 4096

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint8_t que[MAX_QUEUE_LENGTH];
  int front_ptr;
  int back_ptr;
  int len;
  bool is_full;
} BQueue;

/*
  Ensure front(), back(), and pop() are not called on empty queues. 
  Push on a full queue will not do anything.
*/

BQueue new_byte_queue(void);
uint8_t front(BQueue *bque);
uint8_t back(BQueue *bque);
bool push(BQueue *bque, uint8_t byte);
uint8_t pop(BQueue *bque);
int length(BQueue *bque);
bool isFull(BQueue *bque);
bool isEmpty(BQueue *bque);
void try_uart_out(void);

#endif // __BYTE_QUEUE_H__