// Button interrupt app
//
// Use interrupts from buttons to control LEDs

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"

// Pin definitions
#include "nrf52840dk.h"

void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  switch(pin) {
    case BUTTON1: {
      nrf_gpio_pin_toggle(LED1);
      break;
    }
    case BUTTON2: {
      nrf_gpio_pin_toggle(LED2);
      break;
    }
    case BUTTON3: {
      nrf_gpio_pin_toggle(LED3);
      break;
    }
    case BUTTON4: {
      nrf_gpio_pin_toggle(LED4);
      break;
    }
  }
}

static void gpio_in_with_interrupt(nrfx_gpiote_pin_t pin, nrf_gpio_pin_pull_t pull, nrfx_gpiote_evt_handler_t handler) {
  // Configure I/O pin
  // HITOLO: Sense high-to-low transition
  // Alternatives: LOTOHI or TOGGLE (for either)
  nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  in_config.pull = pull;
  ret_code_t err_code = nrfx_gpiote_in_init(pin, &in_config, handler);
  APP_ERROR_CHECK(err_code);

  // Enable the interrupts
  nrfx_gpiote_in_event_enable(pin, true);
}

int main(void) {

  // Initialize LEDs
  nrf_gpio_cfg_output(LED1);
  nrf_gpio_cfg_output(LED2);
  nrf_gpio_cfg_output(LED3);
  nrf_gpio_cfg_output(LED4);
  nrf_gpio_pin_set(LED1);
  nrf_gpio_pin_set(LED2);
  nrf_gpio_pin_set(LED3);
  nrf_gpio_pin_set(LED4);

  // Initialize GPIO task/event (interrupts)
  ret_code_t err_code = nrfx_gpiote_init();
  APP_ERROR_CHECK(err_code);

  gpio_in_with_interrupt(BUTTON1, NRF_GPIO_PIN_PULLUP, button_handler);
  gpio_in_with_interrupt(BUTTON2, NRF_GPIO_PIN_PULLUP, button_handler);
  gpio_in_with_interrupt(BUTTON3, NRF_GPIO_PIN_PULLUP, button_handler);
  gpio_in_with_interrupt(BUTTON4, NRF_GPIO_PIN_PULLUP, button_handler);

  // Enter main loop
  while (true) {
    // do nothing
  }
}

