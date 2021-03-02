// BLE Scanning app
//
// Receives BLE advertisements

#include "app_timer.h"
#include "ble_advertising.h"
#include "simple_ble.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf52840dk.h"
#include "ring_buffer.h"

#define MESH_MESSAGE_OFFSET 10
static uint8_t ble_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX] = {
    2, 0x01,
    6, 0x06, 0x09, 'R', 'E', 'L', 'A', 'Y',
    5, 0x2A, 0, 0, 0, 0
};

static queue_t adv_queue = {
    .data = { 0 },
    .head = 0,
    .tail = 0,
};

// Intervals for advertising and connections
#define ADV_INTERVAL 20 // min 20 ms
int BLE_ADV_TIMEOUT = ADV_INTERVAL * 8.5 / 10; // units of 10 ms
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
    char name[31] = { 0 };

    char peer_address[6] = { 0 };
    uint16_t device_id = adv_report->peer_addr.addr[0] | (adv_report->peer_addr.addr[1] << 8);
    for (int i = 0; i < 6; i++) {
        peer_address[i] = adv_report->peer_addr.addr[i];
        device_id |= adv_report->peer_addr.addr[i] << (8 * i);
    }

    // printf("peer address : %2X \n", device_id);
    uint8_t name_offset = field_start(adv_buf, 0x09, adv_len);
    for (int i = 0; i < adv_buf[name_offset - 1]; i++) {
        name[i] = adv_buf[name_offset + 1 + i];
    }

    uint8_t message_offset = field_start(adv_buf, 0x2A, adv_len);
    uint8_t message_len = adv_buf[message_offset-1];
    uint8_t message[31] = {0};
    for (int i = 0; i < message_len; i++) {
        message[i] = adv_buf[message_offset + 1 + i];
    }
    


    if (device_id == 0xCAFE) {
        printf("This is my root device: CAFE\nlen: %d, data \n", adv_len, adv_len, adv_buf);
        for (uint8_t i = 0; i < adv_len; i++) {
            printf("0x%02x, ", adv_buf[i]);
        }
        printf("\n\n");
        memcpy(ble_data + MESH_MESSAGE_OFFSET, adv_buf + message_offset-1, message_len+1);
        printf("my data: \n");
        for (uint8_t i = 0; i < 31; i++) {
            printf("0x%02x, ", ble_data[i]);
        }
        printf("\n\n");
        advertising_stop();
        copy_adv_data(ble_data, message_len + 11);
        advertising_start();
        simple_ble_adv_raw(ble_data, 31);
        advertising_start();

    }
}

int main(void)
{
    nrf_gpio_cfg_output(LED1);
    app_timer_init();
    // Setup BLE
    // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
    simple_ble_app = simple_ble_init(&ble_config);
    // Start Advertising

    simple_ble_adv_raw(ble_data, 16);
    advertising_start();
    // Start scanning
    scanning_start();
    // go into low power mode
    while (1) {
        power_manage();
    }
}
