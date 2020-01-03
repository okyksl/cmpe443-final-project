#include "External.h"

uint32_t rotation_counter = 0;

void External_Init() {
  IOCON_SPEED_SENSOR |= (1 << IOCON_SPEED_SENSOR_FUNC) // configure IOCON func
  EINT->EXTMODE |= (1 << 0) // edge-sensitive
  EINT->EXTPOLAR |= (1 << 0) // rising-edge

  // enable interrupt for EINT0_IRQn
  NVIC_EnableIRQ(EINT0_IRQn);
  NVIC_SetPriority(EINT0_IRQn, SPEED_SENSOR_PRIORITY);
  NVIC_ClearPendingIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler() {
  EINT->EXTINT |= (1 << 0) // clear interrupt for EINT0
  rotation_counter += 1;
}