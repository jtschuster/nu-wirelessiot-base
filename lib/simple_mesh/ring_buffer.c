#include "ring_buffer.h"
#include "app_error.h"
#include "debug_print.h"
#include <stdint.h>

// #ifndef RINGBUFFER
// #define RINGBUFFER

#ifndef BITQUEUE
inline __attribute__((always_inline)) void q_push(queue_t* q, uint8_t d)
{
    q->data[q->tail] = d;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
}

inline __attribute__((always_inline)) uint8_t q_pop(queue_t* q)
{
    if (q->head == q->tail) {
        APP_ERROR_HANDLER(123);
    }
    uint8_t data = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    return data;
}

inline __attribute__((always_inline)) uint8_t q_has_data(queue_t* q)
{
    return q->tail != q->head;
}

#else
// static void debug(queue_t* q)
// {
//     DEBUG_PRINT("queue:\n\tptr:%d\n\tdata:%08lX%08lX%08lX%08lX%08lX%08lX%08lX%08lX\n", q->ptr,
//         ((uint32_t*)(q->data))[0],
//         ((uint32_t*)(q->data))[1],
//         ((uint32_t*)(q->data))[2],
//         ((uint32_t*)(q->data))[3],
//         ((uint32_t*)(q->data))[4],
//         ((uint32_t*)(q->data))[5],
//         ((uint32_t*)(q->data))[6],
//         ((uint32_t*)(q->data))[7]);
// }

inline __attribute__((always_inline)) void q_push(queue_t* q, uint8_t reg_num)
{
    q->data[reg_num >> 3] |= 1 << ((reg_num & 0b111) - 1);
    // DEBUG_PRINT("pushing %X to byte number %d:\n", reg_num, reg_num >> 3);
    // DEBUG_PRINT("should set this bit: %8X", 1 << (reg_num & 0b111));
    // debug(q);
}

/** Errors when the queue is empty
 *  Always check the the queue has data first (q_has_data)
 */
inline __attribute__((always_inline)) uint8_t q_pop(queue_t* q)
{
    for (uint16_t i = 0; i < 256; i++) {
        if ((q->data[q->ptr >> 3] & (1 << (q->ptr & 0b111))) != 0) {
            q->data[q->ptr >> 3] &= ~(1 << (q->ptr & 0b111));
            q->ptr = (q->ptr + 1) % 256;
            return q->ptr;
        } else {
            q->ptr = (q->ptr + 1) % 256;
        }
    }
    DEBUG_PRINT("dumb dumb you poped an empty boi");
    APP_ERROR_HANDLER(123);
    return -1;
}

inline __attribute__((always_inline)) uint8_t q_has_data(queue_t* q)
{
    uint8_t accum = 0;
    for (int i = 0; i < 32; i++) {
        accum |= q->data[i];
        // DEBUG_PRINT("i=%d, accum: %X, data: %X, data ptr: %X\n", i, accum, q->data[i], q->data + i);
    }
    // DEBUG_PRINT("checking for q data:\n");
    // debug(q);
    // DEBUG_PRINT("i end up with %d in accum\n", accum);
    return accum != 0;
}
#endif

// #endif