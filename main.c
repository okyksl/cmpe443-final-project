#include "LPC407x_8x_177x_8x.h"

#include <stdio.h> 
#include <string.h> 

#include "Library/Serial.h"
#include "Library/Ultrasonic.h"
#include "Library/ADC.h"
#include "Library/Timer.h"

void ultrasonic_init(){
	Ultrasonic_Init();
	Ultrasonic_Trigger_Timer_Init();
	Ultrasonic_Capture_Timer_Init();
	Ultrasonic_Start_Trigger_Timer();
}

char c[14];
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

void init() {	
	Serial_Init();
	ultrasonic_init();
	Timer_Init();
	ADC_Init();
	ADC_Start();
}

uint32_t time;

void ultrasonic_update(){
	if(ultrasonicSensorNewDataAvailable){
		ultrasonicSensorNewDataAvailable = 0;
		time = ultrasonicSensorFallingCaptureTime - ultrasonicSensorRisingCaptureTime;
		time /= 58;
		itoa(time);
		serialTransmitData= c; 
		Serial_WriteData(*serialTransmitData++);
		while(!serialTransmitCompleted);
	}
}

uint32_t adc_value;
void ADC_update(){
	if(ADC_New_Data_Available){
		adc_value = ADC_GetLastValue();
		itoa(adc_value);
		serialTransmitData= c; 
		Serial_WriteData(*serialTransmitData++);
		while(!serialTransmitCompleted);
	}
}

void update() {
	ultrasonic_update();
	ADC_update();
}

int main() {
	init();
	while(1) {
		update();
	}
}

