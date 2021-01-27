// BLE Scanning app
//
// Receives BLE advertisements

#include "simple_ble.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf52840dk.h"

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
    // BLE address is c0:98:e5:4e:00:02
    .platform_id = 0x4E, // used as 4th octet in device BLE address
    .device_id = 0x0002, // used as the 5th and 6th octet in the device BLE address, you will need to change this for each device you have
    .adv_name = "CS397/497", // irrelevant in this example
    .adv_interval = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

uint8_t print_ble_field(uint8_t* data_buff, uint8_t index)
{
    uint8_t field_len = data_buff[index];
    printf("length fo field: %d\n", field_len);
    switch (data_buff[index + 1]) {
    case 0x01:
        printf("field type: flags (0x01)");
        printf("%hx", data_buff[index + 2]);
        return index + 1 + field_len;

    case 0x09:
        printf("field type: local_name");
        printf("%.*s\n", field_len - 1, (char*)data_buff + index + 2);
        return index + 1 + field_len;

    case 0x24:
        printf("field type: URI (0x24)");
        printf("%.*s", field_len - 1, (char*)data_buff + index + 2);
        return index + 1 + field_len;

    default:
        printf("Field type: %hx", data_buff[index + 1]);
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
    while (index < length) {
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
    for (int i = 0; i < 6; i++) {
        peer_address[i] = adv_report->peer_addr.addr[i];
    }

    uint8_t name_offset = field_start(adv_buf, 0x09, adv_len);
    for (int i = 0; i < adv_buf[name_offset - 1]; i++) {
        name[i] = adv_buf[name_offset + 1 + i];
    }

    if (!strncmp("CS397/497", name, 9)) {
        printf("this is my device, advert len %d\n", adv_len);
        print_ble_adv_fields(adv_buf, adv_len);
        printf("Received an advertisement!asdf\n");
    }

    uint16_t los_UUID = 0;
    uint8_t UUID_ind = field_start(adv_buf, 0x03, adv_len);
    if (UUID_ind != 255) {
        los_UUID = adv_buf[UUID_ind + 1] << 8 | adv_buf[UUID_ind + 2];
    }
    if (los_UUID == 0xAAFE) { // EddyStone
        for (uint8_t i = 0; i < adv_len; i++) {
            printf("%x, %c \n", adv_buf[i], adv_buf[i]);
        }
    }

    if (!strncmp("\x97\x03\x4e\xe5\x98\xc0", peer_address, 6)) {
        printf("%s\n\n", adv_buf + 6); // again hard coded isn't ideal but I don't want to spend too much time writing perfect code here
    }
}

int main(void)
{

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
