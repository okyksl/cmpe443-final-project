#ifndef MOTOR_H
#define MOTOR_H

#include "PWM.h"

// PWM used for Motor
#define PWM_MOTOR PWM1
#define PWM_MOTOR_PCONP PWM1_PCONP

// P1.23 (P6) - Right Motor Enable
#define PWM_MOTOR_R_MR MR4
#define PWM_MOTOR_R_CHANNEL 4
#define IOCON_MOTOR_R_FUNC 0x2
#define IOCON_MOTOR_R_ADDRESS 0x4002C0DC
#define IOCON_MOTOR_R *((volatile uint32_t*)(IOCON_MOTOR_R_ADDRESS))

// P1.24 (P5) - Left Motor Enable
#define PWM_MOTOR_L_MR MR5
#define PWM_MOTOR_L_CHANNEL 5
#define IOCON_MOTOR_L_FUNC 0x2
#define IOCON_MOTOR_L_ADDRESS 0x4002C0E0
#define IOCON_MOTOR_L *((volatile uint32_t*)(IOCON_MOTOR_L_ADDRESS))

// P5.2 (P32) and P5.3 (P31) - Right Motor Ins
#define GPIO_MOTOR_R_PORT_A PORT5
#define GPIO_MOTOR_R_PIN_A 2
#define GPIO_MOTOR_R_PORT_B PORT5
#define GPIO_MOTOR_R_PIN_B 3

// P1.20 (P7) and P0.21 (P8) - Left Motor Ins
#define GPIO_MOTOR_L_PORT_A PORT1
#define GPIO_MOTOR_L_PIN_A 20
#define GPIO_MOTOR_L_PORT_B PORT0
#define GPIO_MOTOR_L_PIN_B 21

// TODO: Experiment with these values
// Motor variables
#define MOTOR_DRIVE_CW 0
#define MOTOR_DRIVE_SPEED 100
#define MOTOR_DRIVE_BACK_CW 1
#define MOTOR_ROTATE_SPEED 70
#define MOTOR_ROTATE_COUNT 6

void Motor_Init();
void Motor_Set_Rate(uint32_t PERIOD_IN_MS);
void Motor_Set_Speed(uint32_t R_ON, uint32_t L_ON);
void Motor_Run(uint32_t R_CW, uint32_t L_CW);
void Motor_Drive(uint32_t T_ON);
void Motor_Drive_Back(uint32_t T_ON);
void Motor_Rotate(uint32_t IS_CW);
void Motor_Stop();

// TODO: Need to handle "turning" as well

#endif
