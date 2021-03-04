#include <stdint.h>

#define QUEUE_SIZE 32
#define BITQUEUE 1

#ifndef BITQUEUE
typedef struct queue_s {
    uint8_t data[QUEUE_SIZE][31];
    uint8_t head;
    uint8_t tail;
} queue_t;
#else
typedef struct queue_s {
    uint8_t data[8];
    uint8_t ptr;
} queue_t;
#endif

inline __attribute__((always_inline)) void q_push(queue_t q, uint8_t d);

inline __attribute__((always_inline)) uint8_t q_pop(queue_t q);

