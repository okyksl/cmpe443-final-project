#include "PWM.h"
#include "GPIO.h"

void PWM0_Init() {
	GPIO_DIR_Write(PORT1, (1<<3|1<<5), 1);
	//Change the function of the pin in here:
	IOCON_MOTOR_SPEED |= (0x3 << 0);
	
	PCONP |= (1 << 5 | 1 << 6);
	
	//Enable PWM output for corresponding pin.
	PWM0->PCR |= (1<<9);
	
	PWM0->TCR = 1 << 1;
	
	PWM0->PR = 9;
	
	//Configure MR0 register for a period of 20 ms
	PWM0->MR0 = 120000;
	
	PWM0->MCR = 1 << 1;;
	
	PWM0->LER |= 1 << 0;
	
	PWM0->TCR = (1 << 0 | 1 << 3);
	
	PWM0_Write(0);
}

void PWM0_Cycle_Rate(uint32_t period_In_Ms) {
	//Write a formula that changes the MR0 register value for a given parameter.
	PWM0->MR0 = 6000*period_In_Ms;
	PWM0->LER |= 1 << 0;
}

void PWM0_Write(uint32_t T_ON) {	
	if(T_ON > 100) {
		T_ON = 100;
	}
	
	//Write a formula to calculate the match register of the PWM pin.
	PWM0->MR1 = PWM0->MR0/100*T_ON;
	
	//Enable PWM Match Register Latch.
	PWM0->LER |= 1 << 1;
}

void PWM0_Set_ClockWise(){
	GPIO_PIN_Write(PORT1, (1<<3), 0);
	GPIO_PIN_Write(PORT1, (1<<5), 1);
}
void PWM0_Set_CounterClockWise(){
	GPIO_PIN_Write(PORT1, (1<<3), 1);
	GPIO_PIN_Write(PORT1, (1<<5), 0);
}
void PWM0_Stop(){
	GPIO_PIN_Write(PORT1, (1<<3), 0);
	GPIO_PIN_Write(PORT1, (1<<5), 0);
}
