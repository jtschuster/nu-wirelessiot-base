#include "ring_buffer.h"
#include "app_error.h"
#include <stdint.h>

#ifndef BITQUEUE
inline __attribute__((always_inline)) void q_push(queue_t q, uint8_t d)
{
    q.data[q.tail] = d;
    q.tail = (q.tail + 1) % QUEUE_SIZE;
}

inline __attribute__((always_inline)) uint8_t q_pop(queue_t q)
{
    if (q.head == q.tail) {
        APP_ERROR_HANDLER(123);
    }
    uint8_t data = q.data[q.head];
    q.head = (q.head + 1) % QUEUE_SIZE;
    return data;
}

inline __attribute__((always_inline)) uint8_t q_has_data(queue_t q)
{
    return q.tail != q.head;
}

#else
inline __attribute__((always_inline)) void q_push(queue_t q, uint8_t reg_num)
{
    q.data[reg_num >> 3] |= 1 << (reg_num & 0b111);
}

/** Errors when the queue is empty
 *  Always check the the queue has data first (q_has_data)
 */
inline __attribute__((always_inline)) uint8_t q_pop(queue_t q)
{
    for (uint8_t i = 0; i < 256; i++) {
        if (q.data[q.ptr >> 3] & 1 << (q.ptr & 0b111) != 0) {
            q.ptr = (q.ptr + 1) % 256;
            return q.ptr;
        } 
        else {
            q.ptr = (q.ptr + 1) % 256;
        }
    }
    APP_ERROR_HANDLER(123);
}

inline __attribute__((always_inline)) uint8_t q_has_data(queue_t q)
{
    uint8_t accum = 0;
    for (int i = 0; i < 8; i++) {
        accum |= q.data[i];
    }
    return accum != 0;
}
#endif