// printf app
//
// Use RTT to print messages via printf

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"

#include "nrf52840dk.h"

int main(void) {
  
  nrf_gpio_cfg_output(LED1);

  printf("Board started!\n");

  // loop forever
  uint32_t i = 0;
  while (1) {
    nrf_delay_ms(1000);
    printf("Iteration: %lu\n", i++);
    nrf_gpio_pin_toggle(LED1);
  }
}

