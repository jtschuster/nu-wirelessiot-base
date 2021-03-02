#include <stdint.h>
#include "ring_buffer.h"


void q_push(queue_t q, uint8_t d)
{
    q.data[q.tail] = d;
    q.tail = (q.tail + 1) % QUEUE_SIZE;
}

uint8_t q_pop(queue_t q)
{
    uint8_t data = q.data[q.head];
    q.head = (q.head + 1) % QUEUE_SIZE;
    return data;
}