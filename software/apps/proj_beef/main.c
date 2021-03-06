// BLE Scanning app
//
// Receives BLE advertisements

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


uint8_t ble_data[31] = {0};

// Intervals for advertising and connections
#define ADV_INTERVAL 20 // min 20 ms
int BLE_ADV_TIMEOUT = (ADV_INTERVAL+5) * 7.5 / 10; // units of 10 ms
int TX_POWER_LEVEL = 0;
int APP_ADV_INTERVAL = MSEC_TO_UNITS(ADV_INTERVAL, UNIT_0_625_MS);
static simple_ble_config_t ble_config = {
    // BLE address is c0:98:e5:4e:00:02
    .platform_id = 0x4E, // used as 4th octet in device BLE address
    .device_id = 0xBEEF, // used as the 5th and 6th octet in the device BLE address, you will need to change this for each device you have
    .adv_name = "RELAY",
    .adv_interval = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

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

// Callback handler for advertisement reception
void ble_evt_adv_report(ble_evt_t const* p_ble_evt)
{
    // extract the fields we care about
    ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
    // uint8_t const* ble_addr = adv_report->peer_addr.addr; // array of 6 bytes of the address
    uint8_t* adv_buf = adv_report->data.p_data; // array of up to 31 bytes of advertisement payload data
    uint16_t adv_len = adv_report->data.len; // length of advertisement payload data
    if (adv_len == 31 && adv_buf[1] == 0x2A) {
        // handler can handle it
        mesh_handle_message(adv_buf);
    }

    uint16_t device_id = adv_report->peer_addr.addr[0] | (adv_report->peer_addr.addr[1] << 8);
    for (int i = 0; i < 2; i++) {
        device_id |= adv_report->peer_addr.addr[i] << (8 * i);
    }

    // uint8_t name_offset = field_start(adv_buf, 0x09, adv_len);
    // char name[31] = { 0 };
    // for (int i = 0; i < adv_buf[name_offset - 1]; i++) {
    //     name[i] = adv_buf[name_offset + 1 + i];
    // }

    uint8_t message_offset = field_start(adv_buf, 0x2A, adv_len);
    uint8_t message_len = adv_buf[message_offset-1];
    // uint8_t message[31] = {0};
    // for (int i = 0; i < message_len; i++) {
    //     message[i] = adv_buf[message_offset + 1 + i];
    // }
    


    // if (device_id == 0xCAFE) {
    //     DEBUG_PRINT("This is my root device: CAFE\nlen: %d, data %.*s\n", adv_len, adv_len, adv_buf);
    //     for (uint8_t i = 0; i < adv_len; i++) {
    //         DEBUG_PRINT("0x%02x, ", adv_buf[i]);
    //     }
    //     DEBUG_PRINT("\n\n");
    //     DEBUG_PRINT("my data: \n");
    //     for (uint8_t i = 0; i < 31; i++) {
    //         DEBUG_PRINT("0x%02x, ", ble_data[i]);
    //     }
    //     DEBUG_PRINT("\n\n");
    //     advertising_stop();
    //     copy_adv_data(ble_data, message_len + 11);
    //     advertising_start();
    //     // simple_ble_adv_raw(ble_data, 31);
    //     // advertising_start();

    // }
}
uint8_t reg2data[24] = {0};
void broadcast_next_message() {
    if(mesh_assemble_next_message(ble_data) == 0) {
        DEBUG_PRINT("about to send reg %d = %d\n", ble_data[REG_NUM_OFFSET], ble_data[30]);
        advertising_stop();
        copy_adv_data(ble_data, 31);
        advertising_start();
    } else {
        DEBUG_PRINT("not sending yet\n");
    }
}

void set_leds() {
    uint8_t reg_num = 1;
    uint8_t reg_val[24] = {0};
    mesh_read_reg(reg_val, reg_num);
    DEBUG_PRINT("reg1 is %d\n", reg_val[23]);

    if (reg_val[23] == 0) {
        nrf_gpio_pin_set(LED1);
    } else {
        nrf_gpio_pin_clear(LED1);
    }
    reg_num = 2;
    mesh_read_reg(reg_val, reg_num);
    DEBUG_PRINT("reg2 is %d\n", reg_val[23]);
    if (reg_val[23] == 0) {
        nrf_gpio_pin_set(LED2);
    } else {
        nrf_gpio_pin_clear(LED2);
    }
}

void timer_timeout_handler(void* p_context)
{
    UNUSED_PARAMETER(p_context);
    static uint8_t count = 0;
    static uint8_t rev = 0;
    if (count % 13 == 0) {
        reg2data[23] = reg2data[23] ^ 1;
        mesh_write_reg(1, reg2data, ++rev);
    }
    count++;
    set_leds();
    broadcast_next_message();
}



int main(void)
{
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    app_timer_init();
    // Setup BLE
    // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
    simple_ble_app = simple_ble_init(&ble_config);
    // Start Advertising
    timers_init();
    application_timers_start(300);
    mesh_copy_message(ble_data, 0);
    simple_ble_adv_raw(ble_data, 31);
    advertising_start();
    
    // Start scanning
    scanning_start();
    // go into low power mode
    while (1) {
        power_manage();
    }
}
