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

// --- GLOBALS ---

#define LDR_LIGHT_LIMIT 1700
#define P_METER_MAX_VALUE 4096

#define ULTRASONIC_MIN_DIST 18
#define ULTRASONIC_MEAN_DIST 25
#define ULTRASONIC_MAX_DIST 32

#define HALF_ROTATE_COUNT 6

#define MOTOR_DRIVE_SPEED 90
#define MOTOR_UPDATE_SPEED 5
#define MOTOR_LOW_SPEED 50
#define MOTOR_ANGLE_SPEED 5

uint8_t isTest = 1;
uint8_t isStop = 1;
uint8_t isLight = 0;

uint32_t left_ldr = 0;
uint32_t right_ldr = 0;
uint32_t p_meter = 0;

uint32_t ultrasonic_prev = 0;
uint32_t ultrasonic_dist = 0;

uint32_t rotation_counter = 0;

uint32_t motor_speed = 0;
uint32_t motor_direction = 0;
uint32_t motor_left = 100;
uint32_t motor_right = 100;

// --- GLOBALS ---

// --- HIGH LEVEL CONTROL ---


// stops car until a new command arrives
void stop() {
	Motor_Stop();
	isStop = 1;
}

// drives car in given direction
void drive(uint32_t dir, uint32_t speed) {
	if (dir == 0) {
		Motor_Drive(speed);
	} else {
		Motor_Drive_Back(speed);
	}

	motor_speed = speed;
	motor_direction = dir;
	isStop = 0;
}

// controls light condition and starts & stops car accordingly
void control_light() {
	if (left_ldr > LDR_LIGHT_LIMIT || right_ldr > LDR_LIGHT_LIMIT) { //wtf
		isLight = 1;

		if (!isTest) { // detected finish line
			HM10_SendCommand("FINISH\r\n");
			stop(); // permanently stop motor
		} else {
			Motor_Stop(); // temporarily stop motor
		}
	} else if (isLight && left_ldr < LDR_LIGHT_LIMIT && right_ldr < LDR_LIGHT_LIMIT) {
		isLight = 0;

		// if there was a valid action restore it
		if (!isStop) {
			drive(motor_direction, motor_speed);
		}
	}
}


// rotates car by 90 degrees in desired direction
void rotate(uint32_t dir, uint32_t speed) {
	// start counting wheel rotation
	rotation_counter = 0;
  NVIC_EnableIRQ(EINT0_IRQn);

  stop();
  if (dir == 0) {
  	Motor_Rotate_Right(speed);
  } else {
  	Motor_Rotate_Left(speed);
  }
	while (rotation_counter < HALF_ROTATE_COUNT);
	stop();

	// stop counting wheel rotation
  NVIC_DisableIRQ(EINT0_IRQn);
}



// autodrives the car
void autodrive() {
	// slow down if too right or too left
	if (ultrasonic_dist > ULTRASONIC_MAX_DIST || ultrasonic_dist < ULTRASONIC_MIN_DIST) {
		motor_speed -= MOTOR_UPDATE_SPEED;
		if (motor_speed < MOTOR_LOW_SPEED) {
			motor_speed = MOTOR_LOW_SPEED;
		}
	} else {
		motor_speed += MOTOR_UPDATE_SPEED;
		if (motor_speed > MOTOR_DRIVE_SPEED) {
			motor_speed = MOTOR_DRIVE_SPEED;
		}
	}

	// adjust motor impulse to match desired rotation
	if (ultrasonic_dist > ultrasonic_prev) {
		motor_left = 100;
		motor_right = 100 - MOTOR_ANGLE_SPEED * (ultrasonic_dist - ultrasonic_prev);
	} else if (ultrasonic_dist < ultrasonic_prev) {
		motor_left = 100 - MOTOR_ANGLE_SPEED * (ultrasonic_prev - ultrasonic_dist);
		motor_right = 100;
	}

	Motor_Set_Speed(motor_speed * motor_left / 100, motor_speed * motor_right / 100);
}

// --- HIGH LEVEL CONTROL ---

// --- LOW LEVEL UPDATES ---

void Ultrasonic_Update() {
	// read ultrasonic value
	if (ultrasonicSensorNewDataAvailable) {
		ultrasonicSensorNewDataAvailable = 0;
		ultrasonic_prev = ultrasonic_dist;
		ultrasonic_dist = ((ultrasonicSensorFallingCaptureTime - ultrasonicSensorRisingCaptureTime) % 60000) / 58;
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

		motor_speed = MOTOR_DRIVE_SPEED * p_meter / P_METER_MAX_VALUE;
		if (isTest) { // set speed directly if in test mode
			Motor_Set_Speed(motor_speed, motor_speed);
		}
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
			if (strstr(incoming_message, "STOP")) {
				HM10_SendCommand(incoming_message);
				stop();
			}	else if (strstr(incoming_message, "FORWARD")) {
				HM10_SendCommand(incoming_message);
				drive(0, motor_speed);
			} else if (strstr(incoming_message, "RIGHT")) {
				HM10_SendCommand(incoming_message);
				rotate(0, motor_speed);
			} else if (strstr(incoming_message, "LEFT")) {
				HM10_SendCommand(incoming_message);
				rotate(1, motor_speed);
			} else if (strstr(incoming_message, "BACK")) {
				HM10_SendCommand(incoming_message);
				drive(1, motor_speed);
			} else if (strstr(incoming_message, "AUTO")) {
				HM10_SendCommand(incoming_message);
				stop();
				isTest = 0;
				HM10_SendCommand("AUTONOMOUS\r\n");
			}
		} else {
			if (strstr(incoming_message, "TEST")) {
				HM10_SendCommand(incoming_message);
				stop();
				isTest = 1;
				HM10_SendCommand("TESTING\r\n");
			} else if(strstr(incoming_message, "START")) {
				drive(0, motor_speed);
				HM10_SendCommand(incoming_message);
			}
		}
	}
}

// --- LOW LEVEL UPDATES ---


// --- LIFE CYCLE --

void init() {
	Serial_Init(); // initialize UART
	HM10_Init(); // initialize HM10
	ADC_Init(); // initialize ADC (& timer utilized)
	Ultrasonic_Init(); 	// initialize Ultrasonic Sensor (& timers utiized)
	Motor_Init(); // initialize Motor
	Led_Init();  // initialize LEDs
	External_Init(); // initialize Rotation Counter
}

void update() {
	ADC_Update(); // update ADC read values
	Ultrasonic_Update(); // update Ultrasonic read values
	Serial_Update(); // check and response to given commands

	if (!isTest && !isStop) {
		autodrive();
	}
}

int main() {
	init(); // initialize components

	ADC_Start();  // start ADC
	Ultrasonic_Start(); // start Ultrasonic

	Motor_Stop(); // stop Motor

	while(1) {
		update();
	}
}

// --- LIFE CYCLE --
