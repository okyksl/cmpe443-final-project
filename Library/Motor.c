#include "Motor.h"
#include "GPIO.h"
#include "LED.h"

void Motor_Init() {
	// enable output for motor control GPIOs
	GPIO_DIR_Write(GPIO_MOTOR_R_PORT_A, (1 << GPIO_MOTOR_R_PIN_A | 1 << GPIO_MOTOR_R_PIN_B), 1);
  GPIO_DIR_Write(GPIO_MOTOR_L_PORT_B, (1 << GPIO_MOTOR_L_PIN_A | 1 << GPIO_MOTOR_L_PIN_B), 1);

	// configure IOCONs for PWM output
	IOCON_MOTOR_R |= (IOCON_MOTOR_R_FUNC << 0);
	IOCON_MOTOR_L |= (IOCON_MOTOR_L_FUNC << 0);

	// enable Motor PCONP
	PCONP |= (1 << PWM_MOTOR_PCONP);

	// enable PWM output for corresponding channels
	PWM_MOTOR->PCR |= (1 << (8 + PWM_MOTOR_R_CHANNEL)) | (1 << (8 + PWM_MOTOR_L_CHANNEL));

	// reset counter
	PWM_MOTOR->TCR = 1 << 1;

	// set PR such that each PWM cycle corresponds to 1ms
	PWM_MOTOR->PR = PWM_CLOCK_FREQ / 1000;

	// set reset on MR0 match
	PWM_MOTOR->MCR = 1 << 1;

	// initialize rate
	Motor_Set_Rate(20);

	// enable counter and PWM
	PWM_MOTOR->TCR = (1 << 0 | 1 << 3);

	// initialize speed
	Motor_Set_Speed(0,0);
}

void Motor_Set_Rate(uint32_t PERIOD_IN_MS) {
	PWM_MOTOR->MR0 = PERIOD_IN_MS; // set desired rate to MR0
	PWM_MOTOR->LER |= 1 << 0; // update MR0
}

void Motor_Set_Speed(uint32_t R_ON, uint32_t L_ON) {
	if (R_ON > 100) {
		R_ON = 100; // assume T_ON is percentage value between [0, 100]
	}
	R_ON = PWM_MOTOR->MR0 / 100 * R_ON; // calculate desired MR value

	if (L_ON > 100) {
		L_ON = 100; // assume T_ON is percentage value between [0, 100]
	}
	L_ON = PWM_MOTOR->MR0 / 100 * L_ON; // calculate desired MR value

	// set PWM MR values
	PWM_MOTOR->PWM_MOTOR_R_MR = R_ON;
	PWM_MOTOR->PWM_MOTOR_L_MR = L_ON;

	// update PWM MR values
	PWM_MOTOR->LER |= 1 << PWM_MOTOR_R_CHANNEL;
	PWM_MOTOR->LER |= 1 << PWM_MOTOR_L_CHANNEL;
}

void Motor_Run(uint32_t R_CW, uint32_t L_CW) {
	// run motor with opposing currents
	GPIO_PIN_Write(GPIO_MOTOR_R_PORT_A, (1 << GPIO_MOTOR_R_PIN_A), R_CW);
	GPIO_PIN_Write(GPIO_MOTOR_R_PORT_B, (1 << GPIO_MOTOR_R_PIN_B), !R_CW);

	GPIO_PIN_Write(GPIO_MOTOR_L_PORT_A, (1 << GPIO_MOTOR_L_PIN_A), L_CW);
	GPIO_PIN_Write(GPIO_MOTOR_L_PORT_B, (1 << GPIO_MOTOR_L_PIN_B), !L_CW);
}

void Motor_Drive(uint32_t T_ON) {
	Motor_Run(MOTOR_DRIVE_CW, MOTOR_DRIVE_CW);
	Motor_Set_Speed(MOTOR_DRIVE_SPEED, MOTOR_DRIVE_SPEED);
	Led_Front();
}

void Motor_Drive_Back(uint32_t T_ON) {
	Motor_Run(MOTOR_DRIVE_BACK_CW, MOTOR_DRIVE_BACK_CW);
	Motor_Set_Speed(MOTOR_DRIVE_BACK_SPEED, MOTOR_DRIVE_BACK_SPEED);
	Led_Back();
}

void Motor_Rotate(uint32_t IS_CW) {
	Motor_Run(IS_CW, !IS_CW);
	Motor_Set_Speed(MOTOR_ROTATE_SPEED, MOTOR_ROTATE_SPEED);
	Led_Rotate(IS_CW);
}

void Motor_Stop() {
	// disable motor
	GPIO_PIN_Write(GPIO_MOTOR_R_PORT_A, (1 << GPIO_MOTOR_R_PIN_A), 0);
	GPIO_PIN_Write(GPIO_MOTOR_R_PORT_B, (1 << GPIO_MOTOR_R_PIN_B), 0);

	GPIO_PIN_Write(GPIO_MOTOR_L_PORT_A, (1 << GPIO_MOTOR_L_PIN_A), 0);
	GPIO_PIN_Write(GPIO_MOTOR_L_PORT_B, (1 << GPIO_MOTOR_L_PIN_B), 0);

	Led_Stop(); // close leds
}
