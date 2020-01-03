#include "External.h"

uint32_t rotation_counter = 0;

void External_Init() {
  IOCON_SPEED_SENSOR |= (IOCON_SPEED_SENSOR_FUNC << 0); // configure IOCON func
  EXT->EXTMODE |= (1 << SPEED_SENSOR_CHANNEL); // edge-sensitive
  EXT->EXTPOLAR |= (1 << SPEED_SENSOR_CHANNEL); // rising-edge

  // enable interrupt for EINT0_IRQn
  NVIC_EnableIRQ(EINT0_IRQn);
  NVIC_SetPriority(EINT0_IRQn, SPEED_SENSOR_PRIORITY);
  NVIC_ClearPendingIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler() {
  EXT->EXTINT |= (1 << SPEED_SENSOR_CHANNEL); // clear interrupt for EINT0
  rotation_counter += 1;
}