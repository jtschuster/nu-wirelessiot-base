#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "app_timer.h"

#include "nrf_delay.h"
#include "nrf_clock.h"
#include "nrf_timer.h"
#include "nrf_gpio.h"
#ifdef NRF52
#include "nrf52.h"
#include "core_cm4.h"
#endif

APP_TIMER_DEF(m_timer);

/**@brief Function for starting application timers.
 */
void application_timers_start(uint32_t ms)
{
    ret_code_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_timer, APP_TIMER_TICKS(ms), NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
void __attribute__((weak)) timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.
    err_code = app_timer_create(&m_timer,
                                APP_TIMER_MODE_REPEATED,
                                timer_timeout_handler);
    APP_ERROR_CHECK(err_code);
}




