// Buttons app
//
// Use buttons and switches to control LEDs

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

// Pin definitions
#include "nrf52840dk.h"

int led(int which) {
  switch(which) {
    case 0 :
      return LED1;
    case 1 : 
      return LED2;
    case 2 : 
      return LED4;
    case 3 : 
      return LED3;
  }
  return -1;
}

int main(void) {

  // Initialize.
  nrf_gpio_cfg_output(LED1);
  nrf_gpio_cfg_output(LED2);
  nrf_gpio_cfg_output(LED3);
  nrf_gpio_cfg_output(LED4);
  nrf_gpio_pin_set(LED1);
  nrf_gpio_pin_set(LED2);
  nrf_gpio_pin_set(LED3);
  nrf_gpio_pin_set(LED4);
  nrf_gpio_cfg_input(BUTTON1, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON2, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON3, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON4, NRF_GPIO_PIN_PULLUP);

  // Enter main loop.
  uint which = 0; 
  int incr = 1;
  uint speed = 10;

  while (1) {
    for (uint i = 0; i < 50000*speed; i++) {
      if (!nrf_gpio_pin_read(BUTTON1)) {
        nrf_delay_ms(100);
        // incr = -incr;
        speed += 1;
      }
      if (!nrf_gpio_pin_read(BUTTON2)) {
        nrf_delay_ms(100);
        speed = (speed == 1) ? 1 : speed - 1;
      }
    }
    nrf_gpio_pin_set(led(which));
    which = (which + incr) % 4;
    nrf_gpio_pin_clear(led(which));
  }
  return 0;
}

