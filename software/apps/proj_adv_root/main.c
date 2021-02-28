// BLE Advertisement Raw app
//
// Sends a BLE advertisement with raw bytes
#include "app_util.h"
#include "ble_advertising.h"
#include "ble_types.h"
#include "nrf_timer.h"
#include "radio_config.h"
#include "timer.h"

#ifdef NRF52
#include "core_cm4.h"
#include "nrf52.h"
#endif
// #define DEFAULT_TX_POWER RADIO_TXPOWER_TXPOWER_Pos8dBm

#include "simple_ble.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf52840dk.h"

// Intervals for advertising and connections
#define ADV_INTERVAL 20 // min 20 ms
int BLE_ADV_TIMEOUT = ADV_INTERVAL * 8.5 / 10; // units of 10 ms
int TX_POWER_LEVEL = 0;
int APP_ADV_INTERVAL = MSEC_TO_UNITS(ADV_INTERVAL, UNIT_0_625_MS);
static simple_ble_config_t ble_config = {
    // c0:98:e5:4e:xx:xx
    .platform_id = 0x4E, // used as 4th octect in device BLE address
    .device_id = 0xCAFE, // must be unique on each device you program!
    .adv_name = "CS397/497", // used in advertisements if there is room
    .adv_interval = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),
};

/*******************************************************************************
 *   State for this application
 ******************************************************************************/
// Main application state
simple_ble_app_t* simple_ble_app;

uint8_t ble_data[15] = {
    2,  0x01, 0x06, 
    5,  0x09,  'R',  'O',  'O',  'T',
    5,  0x2A, 0x00, 0x00, 0x00, 0x00
};

void timer_timeout_handler(void* p_context)
{
    UNUSED_PARAMETER(p_context);
    printf("asdf %hhd\n", ble_data[14]);
    ble_data[14] += 1;
    advertising_stop();
    // simple_ble_adv_raw(ble_data, 15);
    copy_adv_data(ble_data, 15);
    advertising_start();
}

int main(void)
{
    // init_nvic_irq();
    // setup_timer();
    // nrf_timer_int_enable(NRF_TIMER0,NRF_TIMER_INT_COMPARE0_MASK );
    // nrf_timer_task_trigger(NRF_TIMER0, NRF_TIMER_TASK_START);
    printf("Board started. Initializing BLE: \n");
    // Setup BLE
    // Note: simple BLE is our own library. You can find it in
    // `nrf5x-base/lib/simple_ble/`
    simple_ble_app = simple_ble_init(&ble_config);
    // Start Advertising
    timers_init();
    application_timers_start(5000);

    simple_ble_adv_raw(ble_data, 15);
    printf("Started BLE advertisements\n");

    while (1) {
        power_manage();
    }
}
