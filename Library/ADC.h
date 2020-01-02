#ifndef ADC_H
#define ADC_H

#include "LPC407x_8x_177x_8x.h"

#include "SystemStructures.h"

typedef struct {
  volatile	uint32_t CR;
  volatile	uint32_t GDR;
						uint32_t RESERVED0;
  volatile	uint32_t INTEN;
  volatile	uint32_t DR[8];
  volatile	uint32_t STAT;
  volatile	uint32_t TRM;
} ADC_TypeDef;

#define ADC_CLOCK_FREQUENCY 1000000

//Write the IOCON address of Analog Pin
#define	ANALOG_PIN_IOCON_ADDRESS_L	0x4002C064
#define ANALOG_PIN_IOCON_L	*((volatile uint32_t*)(ANALOG_PIN_IOCON_ADDRESS_L))
	
#define	ANALOG_PIN_IOCON_ADDRESS_R	0x4002C068
#define ANALOG_PIN_IOCON_R	*((volatile uint32_t*)(ANALOG_PIN_IOCON_ADDRESS_R))

//Write the max value of ADC.
#define ADC_MAX_VALUE 0x11111111

//Define a ADC_CLKDIV variable for given ADC_CLOCK_FREQUENCY.
#define ADC_CLKDIV 0x0000003B

#define ADC_BASE	0x40034000
#define ADC	((ADC_TypeDef*) ADC_BASE)

extern uint32_t ADC_Last_L;
extern uint8_t ADC_New_Data_Available_L;
extern uint32_t ADC_Last_R;
extern uint8_t ADC_New_Data_Available_R;

void ADC_Init(void);
void ADC_Start(void);
void ADC_Stop(void);
uint32_t ADC_GetLastValue_L(void);
uint32_t ADC_GetLastValue_R(void);

#endif
