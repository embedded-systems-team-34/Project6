/******************************************************************************
* FILENAME : PWM.c          
*
* DESCRIPTION : 
*     PWM setup and interface code.  Setup PWM for a period of 20 ms on channel_num
*     1 and 2 using TIM5
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/

#include "stm32l476xx.h"

// Function Prototypes
void pwmInit() {
    // set up Tim5 for PWM on channel 1 (PA0) and 2 (PA1)
    
    // Enable clock for timer 5
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
    
    // Set Prescaler
    // 80 MHz / 80 = 1 MHz -> 1 us
    TIM5->PSC = 79;
    
    // Set duty cycle period 
    // 1 us * 20000 = 20 ms
    TIM5->ARR = 19999;
    TIM5->EGR |= TIM_EGR_UG;
    // Write TIM5_CCR1 to control the duty cycle
    // Set a duty cycle of 2 ms
    
    // Set PWM mode 1, channel 1 is active as long as TIMx->CNT < TIMx->CCR1
    TIM5->CCMR1 |= 0x60;
    // Set PWM mode 1, channel 1 is active as long as TIMx->CNT < TIMx->CCR1
    TIM5->CCMR1 |= 0x6000;
    // Enable Output compare on channel 1
    TIM5->CCER |= 1;
    TIM5->CCER |= 0x10;
    TIM5->CR1 |= TIM_CR1_CEN;
}

void setPWMDuty( unsigned int channel_num, uint16_t duty_cycle) {
    // Write new Duty cycle value for channel 1
    if (channel_num == 0) {
        TIM5->CCR1 = duty_cycle;
    }
    // Write new duty cycle value for channel 2
    else {
        TIM5->CCR2 = duty_cycle;
    }
}
