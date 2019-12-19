#include "LPC407x_8x_177x_8x.h"

#include <stdio.h> 
#include <string.h> 

#include "Library/Serial.h"
#include "Library/HM10.h"

char characters[128] = ""; 

void init() {	
	//Serial_Init();
	
	//HM10_Init();
}

void update() {
	
}

int main() {
	init();
	
	while(1) {
		update();
	}
}

