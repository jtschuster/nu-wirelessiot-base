#ifndef TIMERS
#define TIMERS

#include "debug_print.h"
#include <stdint.h>

void timers_init();

void timer_timeout_handler(void* p_context);

void application_timers_start(uint32_t ms);

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "nrf.h"

#include "nrf_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_timer.h"

#ifdef NRF52
#include "core_cm4.h"
#include "nrf52.h"
#endif

#define TICKS_TO_MS(_ticks)                                   \
    ((uint32_t)ROUNDED_DIV(                                   \
        (_ticks)*1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1), \
        (uint64_t)APP_TIMER_CLOCK_FREQ))

APP_TIMER_DEF(m_app_timer);
APP_TIMER_DEF(m_mesh_timer);

/**@brief Function for starting application timers.
 */
void mesh_timer_start(uint32_t ms)
{
    ret_code_t err_code;
    // Start application timers.
    err_code = app_timer_start(m_mesh_timer, APP_TIMER_TICKS(ms), NULL);
    APP_ERROR_CHECK(err_code);
}
void mesh_timer_stop()
{
    ret_code_t err_code;
    // Start application timers.
    err_code = app_timer_stop(m_mesh_timer);
    APP_ERROR_CHECK(err_code);
}

void timer0_start(uint32_t ms)
{
    ret_code_t err_code;
    // Start application timers.
    err_code = app_timer_start(m_app_timer, APP_TIMER_TICKS(ms), NULL);
    APP_ERROR_CHECK(err_code);
}

void timer0_stop()
{
    ret_code_t err_code;
    // Start application timers.
    err_code = app_timer_stop(m_app_timer);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the timer timeout.
 *
 * @details This function will be called each time the timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
void __attribute__((weak)) timer0_timeout_handler(void* p_context);
void __attribute__((weak)) mesh_timer_timeout_handler(void* p_context);

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init()
{
    static uint8_t initted = 0;
    if (!initted) {
        ret_code_t err_code;

        // Initialize timer module.
        err_code = app_timer_init();
        APP_ERROR_CHECK(err_code);

        // Create timers.
        if (timer0_timeout_handler) {
            err_code = app_timer_create(&m_app_timer,
                APP_TIMER_MODE_REPEATED,
                timer0_timeout_handler);
            APP_ERROR_CHECK(err_code);
        }
        if (mesh_timer_timeout_handler) {
            err_code = app_timer_create(&m_mesh_timer,
                APP_TIMER_MODE_SINGLE_SHOT,
                mesh_timer_timeout_handler);
            APP_ERROR_CHECK(err_code);
        }
    }
    initted = 1;
}

#endif