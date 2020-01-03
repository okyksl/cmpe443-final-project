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

#define LIGHT_LIMIT 1700

uint8_t RIGHT_MOTOR_SPEED, RIGHT_MOTOR_DIR, LEFT_MOTOR_SPEED, LEFT_MOTOR_DIR;

char c[14],incoming_message[20], status_info[500];
uint8_t isTestMode = 1;
uint8_t isLight = 0;
uint8_t isStop = 0;
uint32_t rotation_counter = 0;
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

void ultrasonic_init(){
	Ultrasonic_Init();
	Ultrasonic_Trigger_Timer_Init();
	Ultrasonic_Capture_Timer_Init();
	Ultrasonic_Start_Trigger_Timer();
}

void init() {
	Serial_Init();
	HM10_Init();
	ultrasonic_init();
	Timer0_Init();
	ADC_Init();
	ADC_Start();
	Motor_Init();
	Led_Init();
	Motor_Stop();
	External_Init();
	//Motor_Drive(80);
	//Led_Front();
	//Led_Back();
	//Led_Rotate(0);
}

uint32_t dist;
void ultrasonic_update(){
	if(ultrasonicSensorNewDataAvailable){
		ultrasonicSensorNewDataAvailable = 0;
		dist = ultrasonicSensorFallingCaptureTime - ultrasonicSensorRisingCaptureTime;
		
		dist /= 58;
	}
}

uint32_t left_ldr,right_ldr, p_meter;
// TODO decide value of ldr parameters
void ADC_update(){
	if(ADC_New_Data_Available_L){
		left_ldr = ADC_GetLastValue_L();
	}
	if(ADC_New_Data_Available_R){
		right_ldr = ADC_GetLastValue_R();
	}
	if( left_ldr > LIGHT_LIMIT || right_ldr > LIGHT_LIMIT){
		isLight = 1;
		Motor_Stop();
	}
	else if(isLight && left_ldr < LIGHT_LIMIT && right_ldr < LIGHT_LIMIT){
		isLight = 0;
		Motor_Set_Speed(LEFT_MOTOR_SPEED*p_meter/4096, RIGHT_MOTOR_SPEED*p_meter/4096);
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
	if(ADC_New_Data_Available_P){
		p_meter = ADC_GetLastValue_P();
		Motor_Set_Speed(LEFT_MOTOR_SPEED*p_meter/4096, RIGHT_MOTOR_SPEED*p_meter/4096);
		//itoa(adc_value);
		//serialTransmitData= c; 
		//Serial_WriteData(*serialTransmitData++);
		//while(!serialTransmitCompleted);
	}
}

void Communication_Update(){
	uint32_t i=0;
	if(HM10NewDataAvailable){
		HM10NewDataAvailable = 0;
		while( HM10BufferHead != HM10BufferTail ){
			incoming_message[i++] = HM10Buffer[HM10BufferHead++];
			if(HM10Buffer[HM10BufferHead-1] == '\n'){
				incoming_message[i] = '\0';
				break;
			}
		}
		if( HM10BufferTail+20 > HM10BufferSize){
			HM10_ClearBuffer();
		}
		if(isTestMode && strstr(incoming_message, "STOP")){
			HM10_SendCommand(incoming_message);
			isStop = 1;
			Motor_Stop();
		}
		else if(isTestMode && strstr(incoming_message, "FORWARD")){
			RIGHT_MOTOR_DIR = 1;
			LEFT_MOTOR_DIR = 1;
			RIGHT_MOTOR_SPEED = 90;
			LEFT_MOTOR_SPEED = 90;
			isStop = 0;
			HM10_SendCommand(incoming_message);
			Motor_Drive(90*p_meter/4096);
		}
		else if(isTestMode && strstr(incoming_message, "RIGHT")){
			HM10_SendCommand(incoming_message);
			rotation_counter = 0;
			Motor_Rotate(0);
			while (rotation_counter < MOTOR_ROTATE_COUNT);
			Motor_Stop();
		}
		else if(isTestMode && strstr(incoming_message, "LEFT")){
			HM10_SendCommand(incoming_message);
			rotation_counter = 0;
			Motor_Rotate(1);
			while (rotation_counter < MOTOR_ROTATE_COUNT);
			Motor_Stop();
		}
		else if(isTestMode && strstr(incoming_message, "BACK")){
			RIGHT_MOTOR_DIR = 0;
			LEFT_MOTOR_DIR = 0;
			RIGHT_MOTOR_SPEED = 90;
			LEFT_MOTOR_SPEED = 90;
			isStop = 0;
			HM10_SendCommand(incoming_message);
			Motor_Drive_Back(90*p_meter/4096);
		}
		else if(strstr(incoming_message, "STATUS")){
			HM10_SendCommand(incoming_message);
			sprintf(status_info, "{\"distance\":%d,\"light_level_left\":%d,\"light_level_right\":%d,\"op_mode\":%s}\r\n", dist, left_ldr, right_ldr, isTestMode? "\"TEST\"" : "\"AUTO\"");
			HM10_SendCommand(status_info);
		}
		else if(!isTestMode && strstr(incoming_message, "TEST")){
			HM10_SendCommand(incoming_message);
			isTestMode = 1;
			HM10_SendCommand("TESTING\r\n");
		}
		else if(isTestMode && strstr(incoming_message, "AUTO")){
			Motor_Stop();
			isStop = 1;
			HM10_SendCommand(incoming_message);
			isTestMode = 0;
			HM10_SendCommand("AUTONOMOUS\r\n");
		}
		else if(!isTestMode && strstr(incoming_message, "START")){
			isStop = 0;
			HM10_SendCommand(incoming_message);
		}
	}
}

void update() {
	ultrasonic_update();
	ADC_update();
	Communication_Update();
}

int main() {
	init();
	while(1) {
		update();
	}
}

