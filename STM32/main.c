/******************************************************************************
* FILENAME : main.c          
*
* DESCRIPTION : STM32 code to decode a pulse from the QNX box and translate this
*               pulse into a motor position command.
*     
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/

#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "UART.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "timer2.h"
#include "pwm.h"

#define debug (0)

int main(void){
   
    // PA1 motor PWM output
    // PA0 QNX PWM input signal
	
    System_Clock_Init();

#if debug
    LED_Init();
#endif

    UART2_Init();
    
    // Configure Timer 2 Channel 1 For Input Capture to measure QNX pulse length
    timer2PWMInputModeInit();
    // Setup Motor PWM 
	setupPWMAlternateFunction();
	pwmInit();     
    
    // Enable the interrupt handler to measure TIM2 QNX pulse duty cycle
    NVIC_EnableIRQ(TIM2_IRQn);  

    while(1);
    
}
