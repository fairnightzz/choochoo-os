#include "byte_queue.h"
BQueue new_byte_queue(void) {
  BQueue new_queue = {
    .que = {0},
    .front_ptr = 0,
    .back_ptr = 0,
    .len = 0,
  };
  return new_queue;
}

uint8_t front(BQueue *bque) {
  return bque->que[bque->front_ptr];
}

uint8_t back(BQueue *bque) {
  return bque->que[(bque->back_ptr - 1) % MAX_QUEUE_LENGTH];
}

// returns 1 if push successful otherwise 0
bool push(BQueue *bque, uint8_t byte) {
  if (bque->len < MAX_QUEUE_LENGTH) {
    bque->que[bque->back_ptr] = byte;
    bque->back_ptr += 1;
    bque->back_ptr %= MAX_QUEUE_LENGTH;
    bque->len += 1;
    return true;
  }
  bque->is_full = true;
  return false;
}

uint8_t pop(BQueue *bque) {
  uint8_t resp = bque->que[bque->front_ptr];

  bque->front_ptr += 1;
  bque->front_ptr %= MAX_QUEUE_LENGTH;
  bque->len -= 1;

  bque->is_full = false;
  return resp;
}

int length(BQueue *bque) {
  return bque->len;
}

bool isFull(BQueue *bque) {
  return length(bque) == MAX_QUEUE_LENGTH;
}

bool isEmpty(BQueue *bque) {
  return length(bque) == 0;
}