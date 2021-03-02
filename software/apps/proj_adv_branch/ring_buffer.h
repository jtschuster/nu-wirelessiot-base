#include <stdint.h>

#define QUEUE_SIZE 32

typedef struct queue_s {
    uint8_t data[QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
} queue_t;

void q_push(queue_t q, uint8_t d) {
    q.data[q.tail] = d; 
    q.tail = (q.tail + 1) % QUEUE_SIZE;
}

uint8_t q_pop(queue_t q) {
    uint8_t data = q.data[q.head];
    q.head = (q.head + 1) % QUEUE_SIZE;
    return data;
}


    // #define Q_PUSH(q, d) (                      \
//     do {                                    \
//         q.data[q.tail] = d;                 \
//         q.tail = (q.tail + 1) % QUEUE_SIZE; \
//     } while (0))
    // #define Q_POP(q) (                          \
//     do {                                    \
//         q.data[q.head] = d;                 \
//         q.tail = (q.tail + 1) % QUEUE_SIZE; \
//     } while (0))
