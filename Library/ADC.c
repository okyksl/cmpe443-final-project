#include "ADC.h"
#include "Timer.h"

uint32_t ADC_Last_L = 0;
uint8_t ADC_New_Data_Available_L = 0;
uint32_t ADC_Last_R = 0;
uint8_t ADC_New_Data_Available_R = 0;
uint32_t ADC_Last_P = 0;
uint8_t ADC_New_Data_Available_P = 0;


void ADC_Timer_Init() {
	TIMER_PIN_IOCON &= ~(0x07);
	TIMER_PIN_IOCON |= 0x03;

	PCONP |= 1 << 1;

	TIMER0->CTCR = 0x0;

	TIMER0->TCR &= ~(1 << 0);

	TIMER0->TCR |= (1 << 1);

	TIMER0->PR = (PERIPHERAL_CLOCK_FREQUENCY / 1000000) - 1;

	//Toggle the corresponding External Match bit/output when MR1 matches with TC.
	TIMER0->EMR |= (0x3<<6);

	//Reset the TC value whenever MR1 matches it
	TIMER0->MCR |= (1<<4);

	//Configure MR1 as ADC will start every 100 milliseconds (Do not forget you configured ADC when rising edge occurs on TIMER 0 MATCH 1)
	TIMER0->MR1 = 100000;

	TIMER0->TCR &= ~(1 << 1);

	TIMER0->TCR |= (1 << 0);
}


void ADC_Init() {
	//Change the function value of pin to ADC.
	ANALOG_PIN_IOCON_L |= (1<<0);
	ANALOG_PIN_IOCON_L &= ~(1<<1 | 1<<2);

	//Change the mode value of pin to mode which should be selected if Analog mode is used.
	ANALOG_PIN_IOCON_L &= ~(1<<3 | 1<<4);

	//Change Analog/Digital mode of pin to Analog.
	ANALOG_PIN_IOCON_L &= ~(1<<7);

	//Change the function value of pin to ADC.
	ANALOG_PIN_IOCON_R |= (1<<0);
	ANALOG_PIN_IOCON_R &= ~(1<<1 | 1<<2);

	//Change the mode value of pin to mode which should be selected if Analog mode is used.
	ANALOG_PIN_IOCON_R &= ~(1<<3 | 1<<4);

	//Change Analog/Digital mode of pin to Analog.
	ANALOG_PIN_IOCON_R &= ~(1<<7);

	//Change the function value of pin to ADC.
	ANALOG_PIN_IOCON_P |= (1<<0 | 1<<1);
	ANALOG_PIN_IOCON_P &= ~(1<<2);

	//Change the mode value of pin to mode which should be selected if Analog mode is used.
	ANALOG_PIN_IOCON_P &= ~(1<<3 | 1<<4);

	//Change Analog/Digital mode of pin to Analog.
	ANALOG_PIN_IOCON_P &= ~(1<<7);

	//Turn on ADC.
	PCONP |= (1<<12);

	//Set the CLKDIV and make the A/D converter operational without Burst mode.
	ADC->CR |= ADC_CLKDIV << 8;

	//Make the A/D converter operational
	ADC->CR |= (1<<21);

	//Make sure conversions are software controlled and require 31 clocks (Do not use Burst mode)
	ADC->CR &= ~(1<<16);

	//Configure CR SEL bits for sampled and converting corresponding pin.
	ADC->CR |= (1<<2);
	ADC->CR |= (1<<3);
	ADC->CR |= (1<<4);

	//Enable interrupt for corresponding pin.
	ADC->INTEN |= (1<<2);
	ADC->INTEN |= (1<<3);
	ADC->INTEN |= (1<<4);

	// Enable Timer
	ADC_Timer_Init();

	//Enable ADC_IRQn (Interrupt Request).
	NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_Start() {
	//Write a code for starting A/D conversion on a rising edge on the TIMER 0 MATCH 1.
	ADC->CR |= (0x4<<24);
	ADC->CR &= ~(1<<27);
}

uint32_t ADC_GetLastValue_L() {
	ADC_New_Data_Available_L = 0;
	return ADC_Last_L;
}

uint32_t ADC_GetLastValue_R() {
	ADC_New_Data_Available_R = 0;
	return ADC_Last_R;
}

uint32_t ADC_GetLastValue_P() {
	ADC_New_Data_Available_P = 0;
	return ADC_Last_P;
}

void ADC_IRQHandler() {
	if ((ADC->GDR & (0x3<<24)) == (0x3<<24)) {
		ADC->GDR &= ~((uint32_t)1 << 31);

		//Write the converted data (only the converted data) to ADC_Last variable.
		ADC_Last_R = (ADC->DR[3] >> 4) & 0xfff;

		ADC_New_Data_Available_R = 1;
	} else if ((ADC->GDR & (0x2<<24)) == (0x2<<24)) {
		ADC->GDR &= ~((uint32_t)1 << 31);

		//Write the converted data (only the converted data) to ADC_Last variable
		ADC_Last_L = (ADC->DR[2] >> 4) & 0xfff;

		ADC_New_Data_Available_L = 1;
	} else {
		ADC->GDR &= ~((uint32_t)1 << 31);

		//Write the converted data (only the converted data) to ADC_Last variable.
		ADC_Last_P = (ADC->DR[4] >> 4) & 0xfff;

		ADC_New_Data_Available_P = 1;
	}
}
