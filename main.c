/******************************************************************************
* FILENAME : main.c          
*
* DESCRIPTION : 
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
#include "GPIOA.h"

#define debug (0)


/******************************************************************************
* GLOBAL VARIABLES
******************************************************************************/

int main(void){
    int n;
    unsigned int wasPost;
    char rxbyte;
   
    System_Clock_Init();

#if debug
    LED_Init();
#endif

    UART2_Init();
    gpioAInit();
    
    // Configure Timer 2 Channel 1 For Input Capture
    timer2PWMInputModeInit();
     
    
    // Enable the interrupt handler
    NVIC_EnableIRQ(TIM2_IRQn);     
    
}
