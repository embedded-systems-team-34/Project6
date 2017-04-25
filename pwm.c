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

#define MOTOR_MINIMUM_POS (490)
#define MOTOR_MAXIMUM_POS (1969)
#define MOTOR_NUM_STEPS (MOTOR_MAXIMUM_POS - MOTOR_MINIMUM_POS)

void setupPWMAlternateFunction() {
    // Initialize PWM for channels 1 and two
    // Configure PA0 for alternate function
    // Enable clk to PortA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    // Set PA0 to be alternate function
    GPIOA->MODER &= ~GPIO_MODER_MODER0;        // Clear moder register mode0[1:0]
    GPIOA->MODER |= GPIO_MODER_MODER0_1;    // Set alternate function mode 10
    
    // Set Alternate function lower register to AF2 so that A0 is set connected to TIM5_CH1
    GPIOA->AFR[0] = 0x2;
        
    // Set PA1 to be alternate function
    GPIOA->MODER &= ~GPIO_MODER_MODER1;        // Clear moder register mode0[1:0]
    GPIOA->MODER |= GPIO_MODER_MODER1_1;    // Set alternate function mode 10        
        
    // Set Alternate function lower register to AF2 so that A0 is set connected to TIM5_CH1
    GPIOA->AFR[0] |= 0x20;    
}

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

// Returns a PWM duty cycle value to command corresponding to a normalized
// float between 0 and 1
uint16_t setNormalizedPWMDuty(float norm_pos) {
    return uint16_t(norm_pos * MOTOR_NUM_STEPS) + MOTOR_MINIMUM_POS;
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
