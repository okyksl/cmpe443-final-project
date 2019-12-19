#include "Serial.h"

char serialReceivedCharacter = 0;
uint8_t serialNewDataAvailable = 0;
char* serialTransmitData = 0;
uint8_t serialTransmitCompleted = 0;


void Serial_Init() {
	//Change the function of TX and RX pins for UART.
	BYTE_WRITE(&Serial_UART_TX_PIN, 0x7, 0x1);
	BYTE_WRITE(&Serial_UART_RX_PIN, 0x7, 0x1);
	
	//Turn on UART0.
	BYTE_WRITE(&PCONP, 0x8, 0x8); 
	
	//Enable FIFO for UART0.
	BYTE_WRITE(&(SERIAL_UART->FCR), 0x1, 0x1);
	
	//In order to change the DLM, DLL and FDR values, Write correct code for enabling the access to Divisor Latches.

	
	//Write correct DLM, DLL and FDR values for 115200 baudrate
	
	//Write correct code for disabling the access to Divisor Latches.
	
	//Change LCR register value for 8-bit character transfer, 1 stop bits and Even Parity.
							
	//Enable the Receive Data Available and THRE Interrupt.
	
	//Enable UART0_IRQn Interrupt.
	
	//Set UART0_IRQn Priority to 5.
}

void UART0_IRQHandler() {	
	uint32_t currentInterrupt = 0x00;
	
	//First if statement is for Receive Data Available interrupt.
	//When Receive Data Available, the code will enter to the first if condition.
	if(currentInterrupt == 0x00) {
		serialReceivedCharacter = Serial_ReadData();
		serialNewDataAvailable = 1;
	}
	//Second if statement is for THRE interrupt
	//When THRE interrupt is handled, the code will enter to the second if condition.
	else if(currentInterrupt == 0x00) {
		if(*serialTransmitData > 0) {
			Serial_WriteData(*serialTransmitData++);
		}
		else {
			serialTransmitCompleted = 1;
		}
	}
}

char Serial_ReadData() {
	return Serial_UART->RBR;
}

void Serial_WriteData(const char data) {
	serialTransmitCompleted = 0;
	Serial_UART->THR = data;
}

