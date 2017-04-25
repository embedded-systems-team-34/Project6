/******************************************************************************
* FILENAME : timer2.h          
*
* DESCRIPTION : 
*     Function prototypes for timer2 
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/

#ifndef TIMER_H
#define TIMER_H 

#include "stm32l476xx.h"

// Function Prototypes
// Configure for PWM Input Mode
void timer2PWMInputModeInit(void);
float normalizePWMTime(uint32_t duty_time);

#endif
