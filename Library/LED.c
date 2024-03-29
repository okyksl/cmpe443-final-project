#include "LED.h"

void Led_Init() {
  // configure IOCONs for PWM output
  IOCON_LED_FL |= (IOCON_LED_FL_FUNC << 0);
  IOCON_LED_FR |= (IOCON_LED_FR_FUNC << 0);
  IOCON_LED_BL |= (IOCON_LED_BL_FUNC << 0);
  IOCON_LED_BR |= (IOCON_LED_BR_FUNC << 0);

  // enable Led PCONP
  PCONP |= (1 << PWM_LED_PCONP);

  // enable PWM output for corresponding channels
  PWM_LED->PCR |= (1 << (8 + PWM_LED_FL_CHANNEL)) | (1 << (8 + PWM_LED_FR_CHANNEL)) | (1 << (8 + PWM_LED_BL_CHANNEL)) | (1 << (8 + PWM_LED_BR_CHANNEL));

  // reset counter
  PWM_LED->TCR = 1 << 1;

  // set PR such that each PWM cycle corresponds to 1ms
  PWM_LED->PR = PWM_CLOCK_FREQ / 1000;

  // set reset on MR0 match
  PWM_LED->MCR = 1 << 1;

  // set period to 0.5s
  PWM_LED->MR0 = 500;
	PWM_LED->LER |= (1 << 0);

  // enable counter and PWM
  PWM_LED->TCR = (1 << 0 | 1 << 3);
}

// open only front LEDs
void Led_Front() {
  PWM_LED->PWM_LED_FL_MR = PWM_LED->MR0;
  PWM_LED->PWM_LED_FR_MR = PWM_LED->MR0;
  PWM_LED->PWM_LED_BL_MR = 0;
  PWM_LED->PWM_LED_BR_MR = 0;
	PWM_LED->LER |= (1 << PWM_LED_FL_CHANNEL | 1 << PWM_LED_FR_CHANNEL | 1 << PWM_LED_BL_CHANNEL | 1 << PWM_LED_BR_CHANNEL);
}

// open only back LEDs
void Led_Back() {
  PWM_LED->PWM_LED_FL_MR = 0;
  PWM_LED->PWM_LED_FR_MR = 0;
  PWM_LED->PWM_LED_BL_MR = PWM_LED->MR0;
  PWM_LED->PWM_LED_BR_MR = PWM_LED->MR0;
  PWM_LED->LER |= (1 << PWM_LED_FL_CHANNEL | 1 << PWM_LED_FR_CHANNEL | 1 << PWM_LED_BL_CHANNEL | 1 << PWM_LED_BR_CHANNEL);
}

// left rotation blink LEDs
void Led_Left() {
  PWM_LED->PWM_LED_FL_MR = PWM_LED->MR0 / 2;
  PWM_LED->PWM_LED_FR_MR = 0;
  PWM_LED->PWM_LED_BL_MR = PWM_LED->MR0 / 2;
  PWM_LED->PWM_LED_BR_MR = 0;
  PWM_LED->LER |= (1 << PWM_LED_FL_CHANNEL | 1 << PWM_LED_FR_CHANNEL | 1 << PWM_LED_BL_CHANNEL | 1 << PWM_LED_BR_CHANNEL);
}

// right rotation blink LEDs
void Led_Right() {
  PWM_LED->PWM_LED_FL_MR = 0;
  PWM_LED->PWM_LED_FR_MR = PWM_LED->MR0 / 2;
  PWM_LED->PWM_LED_BL_MR = 0;
  PWM_LED->PWM_LED_BR_MR = PWM_LED->MR0 / 2;
  PWM_LED->LER |= (1 << PWM_LED_FL_CHANNEL | 1 << PWM_LED_FR_CHANNEL | 1 << PWM_LED_BL_CHANNEL | 1 << PWM_LED_BR_CHANNEL);
}

// close LEDs
void Led_Stop() {
  PWM_LED->PWM_LED_FL_MR = 0;
  PWM_LED->PWM_LED_FR_MR = 0;
  PWM_LED->PWM_LED_BL_MR = 0;
  PWM_LED->PWM_LED_BR_MR = 0;
  PWM_LED->LER |= (1 << PWM_LED_FL_CHANNEL | 1 << PWM_LED_FR_CHANNEL | 1 << PWM_LED_BL_CHANNEL | 1 << PWM_LED_BR_CHANNEL);
}
