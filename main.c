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

uint8_t RIGHT_MOTOR_SPEED, RIGHT_MOTOR_DIR, LEFT_MOTOR_SPEED, LEFT_MOTOR_DIR;
uint8_t isTest = 1;
uint8_t isStop = 0;
uint8_t isLight = 0;

uint32_t ultrasonic_dist = 0;
uint32_t left_ldr = 0;
uint32_t right_ldr = 0;
uint32_t p_meter = 0;
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
	if (ultrasonicSensorNewDataAvailable) {
		ultrasonicSensorNewDataAvailable = 0;
		ultrasonic_dist = (ultrasonicSensorFallingCaptureTime - ultrasonicSensorRisingCaptureTime) / 58;
	}
}

void ADC_Update(){
	if (ADC_New_Data_Available_L) {
		left_ldr = ADC_GetLastValue_L();
	}
	if (ADC_New_Data_Available_R) {
		right_ldr = ADC_GetLastValue_R();
	}
	if (left_ldr > LDR_LIGHT_LIMIT || right_ldr > LDR_LIGHT_LIMIT) {
		isLight = 1;
		Motor_Stop();
	} else if (isLight && left_ldr < LDR_LIGHT_LIMIT && right_ldr < LDR_LIGHT_LIMIT) {
		isLight = 0;
		Motor_Set_Speed(LEFT_MOTOR_SPEED*p_meter/P_METER_MAX_VALUE, RIGHT_MOTOR_SPEED*p_meter/P_METER_MAX_VALUE);
		if(!isStop) {
			Motor_Run(!LEFT_MOTOR_DIR, !RIGHT_MOTOR_DIR);
			if(LEFT_MOTOR_DIR && RIGHT_MOTOR_DIR){
				Led_Front();
			}
			else if(LEFT_MOTOR_DIR && !RIGHT_MOTOR_DIR){
				Led_Rotate(1); //TODO
			}
			else if(!LEFT_MOTOR_DIR && RIGHT_MOTOR_DIR){
				Led_Rotate(0);
			}
			else if(!LEFT_MOTOR_DIR && !RIGHT_MOTOR_DIR){
				Led_Back();
			}
		}
	}
	if (ADC_New_Data_Available_P) {
		p_meter = ADC_GetLastValue_P();
		Motor_Set_Speed(LEFT_MOTOR_SPEED*p_meter/P_METER_MAX_VALUE, RIGHT_MOTOR_SPEED*p_meter/P_METER_MAX_VALUE);
	}
}

// TODO: Rewrite
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
				isStop = 1;
				Motor_Stop();
			}	else if (strstr(incoming_message, "FORWARD")) {
				RIGHT_MOTOR_DIR = 1;
				LEFT_MOTOR_DIR = 1;
				RIGHT_MOTOR_SPEED = 90;
				LEFT_MOTOR_SPEED = 90;
				isStop = 0;
				Motor_Drive(90*p_meter/P_METER_MAX_VALUE);
			} else if (strstr(incoming_message, "RIGHT")) {
				rotate(0);
			} else if (strstr(incoming_message, "LEFT")) {
				rotate(1);
			} else if (strstr(incoming_message, "BACK")) {
				RIGHT_MOTOR_DIR = 0;
				LEFT_MOTOR_DIR = 0;
				RIGHT_MOTOR_SPEED = 90;
				LEFT_MOTOR_SPEED = 90;
				isStop = 0;
				Motor_Drive_Back(90*p_meter/P_METER_MAX_VALUE);
			} else if (strstr(incoming_message, "AUTO")) {
				Motor_Stop();
				isStop = 1;
				isTest = 0;
				HM10_SendCommand("AUTONOMOUS\r\n");
			}
		} else {
			if (strstr(incoming_message, "TEST")) {
				HM10_SendCommand(incoming_message);
				isTest = 1;
				HM10_SendCommand("TESTING\r\n");
			} else if(strstr(incoming_message, "START")) {
				isStop = 0;
				HM10_SendCommand(incoming_message);
			}
		}
	}
}

// rotates car by 90 degrees in desired direction
void rotate(uint32_t IS_CW) {
	// start counting wheel rotation
	rotation_counter = 0;
  NVIC_EnableIRQ(EINT0_IRQn);

	Motor_Rotate(IS_CW);
	while (rotation_counter < MOTOR_ROTATE_COUNT);
	Motor_Stop();

	// stop counting wheel rotation
  NVIC_DisableIRQ(EINT0_IRQn);
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

