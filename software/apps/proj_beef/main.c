// BLE Scanning app
//
// Receives BLE advertisements

#define DEVICE_ID 0xBEEF
#define DEBUG__

#include "app_timer.h"
#include "ble_advertising.h"
#include "debug_print.h"
#include "mesh_protocol.h"
#include "nrf_gpio.h"
#include "ring_buffer.h"
#include "simple_ble.h"
#include "timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf52840dk.h"

uint8_t field_start(uint8_t* data_buff, uint8_t type_flag, uint8_t length)
{
    uint8_t index = 0;
    while (index < length) {
        if (data_buff[index + 1] == type_flag) {
            return index + 1;
        }
        index += data_buff[index] + 1;
    }
    return 0xFF;
}

uint8_t reg2data[24] = { 0 };

void set_leds()
{
    uint8_t reg_num = 1;
    uint8_t reg_val[24] = { 0 };
    mesh_read_reg(reg_val, reg_num);
    DEBUG_PRINT("reg1 is %d\n", reg_val[23]);

    if ((reg_val[23] & 1) == 0) {
        nrf_gpio_pin_set(LED1);
    } else {
        nrf_gpio_pin_clear(LED1);
    }
    reg_num = 2;
    mesh_read_reg(reg_val, reg_num);
    DEBUG_PRINT("reg2 is %d\n", reg_val[23]);
    if ((reg_val[23] & 1) == 0) {
        nrf_gpio_pin_set(LED2);
    } else {
        nrf_gpio_pin_clear(LED2);
    }
    reg_num = 3;
    mesh_read_reg(reg_val, reg_num);
    DEBUG_PRINT("reg3 is %d\n", reg_val[23]);
    if ((reg_val[23] & 1) == 0) {
        nrf_gpio_pin_set(LED3);
    } else {
        nrf_gpio_pin_clear(LED3);
    }
}
void mesh_message_recv_callback()
{
    set_leds();
}

void timer0_timeout_handler(void* p_context)
{
    UNUSED_PARAMETER(p_context);
    static uint8_t count = 0;
    static uint8_t rev[12] = {0};
    uint8_t reg_num = (1 + (3 * count)) % 12;
    mesh_read_reg(reg2data, reg_num);
    reg2data[23] = reg2data[23] + 1;
    mesh_write_reg(reg_num, reg2data, ++(rev[reg_num]));
    count++;
    set_leds();
}

int main(void)
{
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_cfg_output(LED3);

    // Setup BLE
    // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
    // Start Advertising
    mesh_init();
    timers_init();
    timer0_start(1600);
    // simple_ble_adv_raw(ble_data, 31);
    // advertising_start();

    // Start scanning
    // go into low power mode
    while (1) {
        power_manage();
    }
}
