// Analog read app
//
// Reads data from an analog input

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "nrf_delay.h"
#include "nrfx_saadc.h"

#include "nrf52840dk.h"

// ADC channels
#define INPUT_CHANNEL 0

// callback for SAADC events
void saadc_callback (nrfx_saadc_evt_t const * p_event) {
  // don't care about adc callbacks
}

// sample a particular analog channel in blocking mode
nrf_saadc_value_t sample_value (uint8_t channel) {
  nrf_saadc_value_t val;
  ret_code_t error_code = nrfx_saadc_sample_convert(channel, &val);
  APP_ERROR_CHECK(error_code);
  return val;
}

int main (void) {
  ret_code_t error_code = NRF_SUCCESS;

  printf("Board started!\n");

  // initialize analog to digital converter
  nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
  saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
  error_code = nrfx_saadc_init(&saadc_config, saadc_callback);
  APP_ERROR_CHECK(error_code);

  // initialize analog inputs
  // configure with 0 as input pin for now
  nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(0);
  channel_config.gain = NRF_SAADC_GAIN1_6; // input gain of 1/6 Volts/Volt, multiply incoming signal by (1/6)
  channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6 Volt reference, input after gain can be 0 to 0.6 Volts

  // specify input pin and initialize that ADC channel
  channel_config.pin_p = NRF_SAADC_INPUT_AIN7; // Pin P0.31
  error_code = nrfx_saadc_channel_init(INPUT_CHANNEL, &channel_config);
  APP_ERROR_CHECK(error_code);

  // loop forever
  while (1) {
    // sample analog inputs
    nrf_saadc_value_t analog_val = sample_value(INPUT_CHANNEL);

    // display results
    printf("value: %d (millivolts)\n", analog_val);
    nrf_delay_ms(250);
  }
}

