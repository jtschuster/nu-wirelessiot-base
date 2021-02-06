// BLE Service example app
//
// Creates a BLE environmental sensing service

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "simple_ble.h"

#include "nrf52840dk.h"

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
  // c0:98:e5:4e:xx:xx
  .platform_id       = 0x4E,    // used as 4th octect in device BLE address
  .device_id         = 0xBABA,
  .adv_name          = "CS397/497", // used in advertisements if there is room
  .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
  .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
  .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),
};

static simple_ble_service_t environmental_sensing_service = {{
  .uuid128 = {0xFB,0x34,0x9B,0x5F,0x80,0x00,0x00,0x80,
              0x00,0x10,0x00,0x00,0x1A,0x18,0x00,0x00}
}};

static simple_ble_char_t temperature_char = {.uuid16 = 0x2A6E};
static int16_t temperature = 2055; // 69 degrees fahreheit

static simple_ble_char_t humidity_char = { .uuid16 = 0x2A6F};
static uint16_t humidity = 3000; // 30% humidity

static simple_ble_char_t elevation_char = { .uuid16 = 0x2A6C };
static int32_t elevation = 18500; // 185 meters
// technically 24 bits wide, adjust in charactistic config

static simple_ble_char_t wind_speed_char = { .uuid16 = 0x2A70 };
static uint16_t wind_speed = 1000; // 10 m/s

static simple_ble_char_t wind_direction_char = { .uuid16 = 0x2A71 };
static uint16_t wind_direction = 31500; // A Northwestern direction

/*******************************************************************************
 *   State for this application
 ******************************************************************************/
// Main application state
simple_ble_app_t* simple_ble_app;

void ble_evt_write(ble_evt_t const* p_ble_evt) {
  printf("Got write to a characteristic!\n");
}

int main(void) {

  printf("Board started. Initializing BLE: \n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&environmental_sensing_service);
  simple_ble_add_characteristic(1, 0, 0, 0,
      sizeof(temperature), (uint8_t*)&temperature,
      &environmental_sensing_service, &temperature_char);
  simple_ble_add_characteristic(1, 0, 0, 0,
      sizeof(humidity), (uint8_t*)&humidity,
      &environmental_sensing_service, &humidity_char);
  simple_ble_add_characteristic(1, 0, 0, 0,
      3, (uint8_t*)&elevation,
      &environmental_sensing_service, &elevation_char);
  simple_ble_add_characteristic(1, 0, 0, 0,
      sizeof(wind_speed), (uint8_t*)&wind_speed,
      &environmental_sensing_service, &wind_speed_char);
  simple_ble_add_characteristic(1, 0, 0, 0,
      sizeof(wind_direction), (uint8_t*)&wind_direction,
      &environmental_sensing_service, &wind_direction_char);
  // Start Advertising
  simple_ble_adv_only_name();

  while(1) {
    power_manage();
  }
}

