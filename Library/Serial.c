#include "Serial.h"

char serialReceivedCharacter = 0;
uint8_t serialNewDataAvailable = 0;
char* serialTransmitData = 0;
uint8_t serialTransmitCompleted = 0;


void Serial_Init() {
	//Change the function of TX and RX pins for UART.
	uint32_t value = Serial_UART_RX_PIN;
	value |= 1<<0;
	value &= ~(1<<1 | 1<< 2);
	Serial_UART_RX_PIN = value;
	
	value = Serial_UART_TX_PIN;
	value |= 1<<0;
	value &= ~(1<<1 | 1<< 2);
	Serial_UART_TX_PIN = value;
	//Turn on UART0.
	PCONP |= 1<<3;
	//Enable FIFO for UART0.
	Serial_UART->FCR |= 1<<0;
	//In order to change the DLM, DLL and FDR values, Write correct code for enabling the access to Divisor Latches.
	Serial_UART->LCR |= 1<<7;
	//Write correct DLM, DLL and FDR values for 115200 baudrate
	Serial_UART->DLM = 0;
	Serial_UART->DLL = 0x13;
	Serial_UART->FDR = 0x05 << 0 | 0x07 << 4;
	//Write correct code for disabling the access to Divisor Latches.
	Serial_UART->LCR &= ~(1<<7);
	//Change LCR register value for 8-bit character transfer, 1 stop bits and Even Parity.
	Serial_UART->LCR |= (1<<0 | 1<<1);
	Serial_UART->LCR &= ~(1<<2);
	Serial_UART->LCR |= (1<<3);
	Serial_UART->LCR |= (1<<4);
	Serial_UART->LCR &= ~(1<<5);
	//Enable the Receive Data Available and THRE Interrupt.
	Serial_UART->IER |= (1<<0 | 1<<1);
	//Enable UART0_IRQn Interrupt.
	NVIC_EnableIRQ(UART0_IRQn);
	//Set UART0_IRQn Priority to 5.
	NVIC_SetPriority(UART0_IRQn, 5);
}

void UART0_IRQHandler() {	
	uint32_t currentInterrupt = (Serial_UART->IIR & (0x7 << 1)) >> 1;
	
	//First if statement is for Receive Data Available interrupt.
	//When Receive Data Available, the code will enter to the first if condition.
	if(currentInterrupt == 0x2) {
		serialReceivedCharacter = Serial_ReadData();
		serialNewDataAvailable = 1;
	}
	//Second if statement is for THRE interrupt
	//When THRE interrupt is handled, the code will enter to the second if condition.
	else if(currentInterrupt == 0x1) {
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

