#include "LPC407x_8x_177x_8x.h"

#include <stdio.h>
#include <string.h>

#include "Library/Serial.h"
#include "Library/Ultrasonic.h"
#include "Library/ADC.h"
#include "Library/Timer.h"
#include "Library/Motor.h"
#include "Library/Led.h"
#include "Library/External.h"
#include "Library/HM10.h"

#define LDR_LIGHT_LIMIT 1700
#define P_METER_MAX_VALUE 4096
#define HALF_ROTATE_COUNT 6
#define MOTOR_DRIVE_SPEED 90
#define MOTOR_ROTATE_SPEED 65

uint8_t isTest = 1;
uint8_t isLight = 0;
uint32_t ultrasonic_dist = 0;
uint32_t left_ldr = 0;
uint32_t right_ldr = 0;
uint32_t p_meter = 0;
uint32_t motor_speed = 0;
uint32_t rotation_counter = 0;

// converts a number into string
char c[14]
void itoa(uint32_t number) {
	uint32_t index = 0;
	uint32_t reverser = 0;
	char temp;
	for(index = 0 ; index < 14; index++){
		c[index] = 0;
	}
	index = 0;
	if(number == 0){
		return;
	}
	while (number > 0) {
		c[index++] = '0' + (number % 10);
		number /= 10;
	}
	index--;
	while( reverser < index-reverser ){
		temp = c[reverser];
		c[reverser] = c[index-reverser];
		c[index-reverser] = temp;
		reverser++;
	}
	c[++index] = '\r';
	c[++index] = '\n';
}

void Ultrasonic_Update() {
	// read ultrasonic value
	if (ultrasonicSensorNewDataAvailable) {
		ultrasonicSensorNewDataAvailable = 0;
		ultrasonic_dist = (ultrasonicSensorFallingCaptureTime - ultrasonicSensorRisingCaptureTime) / 58;
	}
}

void ADC_Update() {
	// read adc values
	if (ADC_New_Data_Available_L) {
		left_ldr = ADC_GetLastValue_L();
	}
	if (ADC_New_Data_Available_R) {
		right_ldr = ADC_GetLastValue_R();
	}
	if (ADC_New_Data_Available_P) {
		p_meter = ADC_GetLastValue_P();
		motor_speed = 100 * p_meter / P_METER_MAX_VALUE
		Motor_Set_Speed(motor_speed, motor_speed);
	}

	// react to lighting condition
	control_light();
}

char incoming_message[20], status_info[500];
void Serial_Update() {
	uint32_t i = 0;
	if (HM10NewDataAvailable) {
		// read incoming message
		HM10NewDataAvailable = 0;
		while (HM10BufferHead != HM10BufferTail) {
			incoming_message[i++] = HM10Buffer[HM10BufferHead++];
			if (HM10Buffer[HM10BufferHead-1] == '\n') {
				incoming_message[i] = '\0';
				break;
			}
		}
		if (HM10BufferTail+20 > HM10BufferSize) {
			HM10_ClearBuffer();
		}

		// response to commands
		if (strstr(incoming_message, "STATUS")) {
			HM10_SendCommand(incoming_message);
			sprintf(status_info, "{\"distance\":%d,\"light_level_left\":%d,\"light_level_right\":%d,\"op_mode\":%s}\r\n", ultrasonic_dist, left_ldr, right_ldr, isTest ? "\"TEST\"" : "\"AUTO\"");
			HM10_SendCommand(status_info);
		} else if (isTest) {
			HM10_SendCommand(incoming_message);

			if (strstr(incoming_message, "STOP")) {
				stop();
			}	else if (strstr(incoming_message, "FORWARD")) {
				drive(0, speed * MOTOR_DRIVE_SPEED)
			} else if (strstr(incoming_message, "RIGHT")) {
				rotate(0, speed * MOTOR_ROTATE_SPEED);
			} else if (strstr(incoming_message, "LEFT")) {
				rotate(1, speed * MOTOR_ROTATE_SPEED);
			} else if (strstr(incoming_message, "BACK")) {
				drive(1, speed * MOTOR_DRIVE_SPEED)
			} else if (strstr(incoming_message, "AUTO")) {
				stop();
				isTest = 0;
				HM10_SendCommand("AUTONOMOUS\r\n");
			}
		} else {
			if (strstr(incoming_message, "TEST")) {
				HM10_SendCommand(incoming_message);
				isTest = 1;
				HM10_SendCommand("TESTING\r\n");
			} else if(strstr(incoming_message, "START")) {
				drive(90*p_meter/P_METER_MAX_VALUE);
				HM10_SendCommand(incoming_message);
			}
		}
	}
}

// controls light condition and starts & stops car accordingly
void control_light() {
	if (left_ldr > LDR_LIGHT_LIMIT || right_ldr > LDR_LIGHT_LIMIT) {
		isLight = 1;
		PCONP &= ~((1 << PWM0_PCONP) | (1 << PWM1_PCONP))
	} else if (isLight && left_ldr < LDR_LIGHT_LIMIT && right_ldr < LDR_LIGHT_LIMIT) {
		isLight = 0;
		PCONP |= (1 << PWM0_PCONP) | (1 << PWM1_PCONP)
	}
}

// drives car in given direction
void drive(uint32_t dir, uint32_t speed) {
	if (dir == 0) {
		Motor_Drive(speed);
	} else {
		Motor_Drive_Back(speed);
	}
}

// rotates car by 90 degrees in desired direction
void rotate(uint32_t dir, uint32_t speed) {
	// start counting wheel rotation
	rotation_counter = 0;
  NVIC_EnableIRQ(EINT0_IRQn);

  if (dir == 0) {
  	Motor_Rotate_Right(speed);
  } else {
  	Motor_Rotate_Left(speed);
  }
	while (rotation_counter < HALF_ROTATE_COUNT);
	Motor_Stop();

	// stop counting wheel rotation
  NVIC_DisableIRQ(EINT0_IRQn);
}

// stops car
void stop() {
	Motor_Stop();
}

void init() {
	Serial_Init();
	HM10_Init();
	ADC_Init();
	ADC_Start();

	// initialize Ultrasonic Sensor (& timers utiized)
	Ultrasonic_Init();

	// initialize Motor
	Motor_Init();

	// initialize LEDs
	Led_Init();

	// initialize Rotation Counter
	External_Init();
}

void update() {
	// update Ultrasonic read values
	Ultrasonic_Update();

	// update ADC read values
	ADC_Update();

	// check and response to given commands
	Serial_Update();
}

int main() {
	// initialize components
	init();

	// start Ultrasonic
	Ultrasonic_Start();

	// stop Motor
	Motor_Stop();

	while(1) {
		update();
	}
}

