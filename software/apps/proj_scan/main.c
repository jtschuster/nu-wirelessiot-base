// BLE Scanning app
//
// Receives BLE advertisements

#include "app_timer.h"
#include "simple_ble.h"
#include "ble_advertising.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf52840dk.h"

void blink(uint32_t led) {
    static uint32_t last_call = 0;
    static uint32_t now;
    
    now = app_timer_cnt_get();
    printf("\nnow, last, diff: %ld, %ld, %ld\n", now, last_call, now-last_call);
    if (now - last_call >= 5000) {
        nrf_gpio_pin_toggle(led);
        last_call = now;    
    }
}

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
    // BLE address is c0:98:e5:4e:00:02
    .platform_id = 0x4E, // used as 4th octet in device BLE address
    .device_id = 0xBEAF, // used as the 5th and 6th octet in the device BLE address, you will need to change this for each device you have
    .adv_name = "CS397/497_2", // irrelevant in this example
    .adv_interval = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

uint8_t print_ble_field(uint8_t* data_buff, uint8_t index)
{
    uint8_t field_len = data_buff[index];
    printf("length of field: %d\n", field_len);
    switch (data_buff[index + 1]) {
    case 0x01:
        printf("field type: flags (0x01)\n");
        printf("%hx\n", data_buff[index + 2]);
        return index + 1 + field_len;

    case 0x09:
        printf("field type: local_name\n\t");
        printf("%.*s\n", field_len - 1, (char*)data_buff + index + 2);
        return index + 1 + field_len;

    case 0x24:
        printf("field type: URI (0x24)\n\t");
        printf("%.*s", field_len - 1, (char*)data_buff + index + 2);
        return index + 1 + field_len;

    default:
        printf("Field type: %hx\n\t", data_buff[index + 1]);
        uint8_t i = 2;
        for (; i < field_len; i++) {
            printf("%hx, ", data_buff[index + i]);
        }
        return i;
    }
}

void print_ble_adv_fields(uint8_t* data_buff, uint8_t length)
{
    uint8_t index = 0;
    while (index < length-1) {
        index = print_ble_field(data_buff, index);
    }
    return;
}

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
    char name[31] = { 0 };

    char peer_address[6] = { 0 };
    uint16_t device_id = adv_report->peer_addr.addr[0] | (adv_report->peer_addr.addr[1] << 8);
    for (int i = 0; i < 6; i++) {
        peer_address[i] = adv_report->peer_addr.addr[i];
        device_id |= adv_report->peer_addr.addr[i] << (8*i);
    }
    // printf("device id : %2X \n", device_id);
    uint8_t name_offset = field_start(adv_buf, 0x09, adv_len);
    for (int i = 0; i < adv_buf[name_offset - 1]; i++) {
        name[i] = adv_buf[name_offset + 1 + i];
    }
    if (device_id == 0xCAFE || device_id == 0xBEEF) {
        printf("This is my device %4x\n", device_id);
        blink(LED1);
        // print_ble_adv_fields(adv_buf, adv_len);
        for(uint8_t i = 0; i < adv_len; i++) {
            printf("0x%02x, ", adv_buf[i]);
        }
        printf("\n\n");
    }
}


int main(void)
{
    nrf_gpio_cfg_output(LED1);
    app_timer_init();
    // Setup BLE
    // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
    simple_ble_app = simple_ble_init(&ble_config);
    advertising_stop();
    // Start scanning
    scanning_start();
    // go into low power mode
    while (1) {
        power_manage();
    }
}
