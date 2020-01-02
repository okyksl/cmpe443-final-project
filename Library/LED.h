#ifndef LED_H
#define LED_H

#include "PWM.h"

#define PWM_LED PWM0
#define PWM_LED_PCONP PWM0_PCONP

// P1.2 (P30) - front right LED
#define PWM_LED_FR_MR MR1
#define PWM_LED_FR_CHANNEL 1
#define IOCON_LED_FR_FUNC 0x3
#define IOCON_LED_FR_ADDRESS 0x4002C088
#define IOCON_LED_FR *((volatile uint32_t*)(IOCON_LED_FR_ADDRESS))

// P1.3 (P29) - front left LED
#define PWM_LED_FL_MR MR2
#define PWM_LED_FL_CHANNEL 2
#define IOCON_LED_FL_FUNC 0x3
#define IOCON_LED_FL_ADDRESS 0x4002C08C
#define IOCON_LED_FL *((volatile uint32_t*)(IOCON_LED_FL_ADDRESS))

// P1.5 (P28) - back right LED
#define PWM_LED_BR_MR MR3
#define PWM_LED_BR_CHANNEL 3
#define IOCON_LED_BR_FUNC 0x3
#define IOCON_LED_BR_ADDRESS 0x4002C094
#define IOCON_LED_BR *((volatile uint32_t*)(IOCON_LED_BR_ADDRESS))

// P1.6 (P27) - back left LED
#define PWM_LED_BL_MR MR4
#define PWM_LED_BL_CHANNEL 4
#define IOCON_LED_BL_FUNC 0x3
#define IOCON_LED_BL_ADDRESS 0x4002C098
#define IOCON_LED_BL *((volatile uint32_t*)(IOCON_LED_BL_ADDRESS))

void Led_Init();
void Led_Front();
void Led_Back();
void Led_Rotate(uint32_t IS_CW);
void Led_Stop();

#endif
