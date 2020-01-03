#ifndef EXTERNAL_H
#define EXTERNAL_H

#include "LPC407x_8x_177x_8x.h"

typedef struct
{
  volatile  uint32_t EXTINT;
            uint32_t RESERVED0[1];
  volatile  uint32_t EXTMODE;
  volatile  uint32_t EXTPOLAR;
} EXT_TypeDef;

// P2.10 (P23)
#define IOCON_SPEED_SENSOR_ADDRESS 0x4002C128
#define IOCON_SPEED_SENSOR_FUNC 0x01
#define IOCON_SPEED_SENSOR *((volatile uint32_t*)(IOCON_SPEED_SENSOR_ADDRESS))
#define SPEED_SENSOR_PRIORITY 99

#define EXT_ADDRESS 0x400FC140
#define EXT ((EXT_TypeDef*) EXT_ADDRESS)

extern uint32_t rotation_counter;
void External_Init(void);

#endif
