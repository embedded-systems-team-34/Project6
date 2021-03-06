/******************************************************************************
* FILENAME : PWM.h          
*
* DESCRIPTION : 
*     Function prototypes for PWM 
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/
#ifndef PWM_H
#define PWM_H 

#include "stm32l476xx.h"

// Function Prototypes
void pwmInit(void);
void setPWMDuty(uint16_t duty_cycle);
void setupPWMAlternateFunction(void);
void setNormalizedPWMDuty(float norm_pos);

#endif
