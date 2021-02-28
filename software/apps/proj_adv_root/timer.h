#include <stdint.h>


void timers_init(void);
void timer_timeout_handler(void * p_context);
void application_timers_start(uint32_t ms);