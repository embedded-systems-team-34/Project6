/******************************************************************************
* FILENAME : timer2.c          
*
* DESCRIPTION : 
*     Timer 2 code to setup timers for various modes of operation.
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/

#include "timer2.h"

uint16_t MAX_PWMTIME

void TIM2_IRQHandler(void) {
    uint16_t which_interrupt = TIM2->SR;
    
    // Input channel 2 capture interrupt
    if ((which_interrupt & TIM_SR_CC1IF) == TIM_SR_CC1IF) {
        // Read the latched time from the capture event
        // this also clears the capture flag in TIM2->SR
        current_rising_edge_count = TIM2->CCR2;
        // Normalize and kick motor PWM logic here
        // TIM2->CCR2 will contain pulse duration in us here!!!
    }
}

// Configure for PWM Input Mode
// See 27.3.6 - PWM Input Mode of the reference manual
void timer2PWMInputModeInit() {
    
    // Enable clock of timer 2
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    
    // Only allow an update event (UIF) when an overflow occurs 
    TIM2->CR1 |= TIM_CR1_URS;
        
    // Set Prescaler
    // 80 MHz / 80 = 1 MHz -> 1 us
    TIM2->PSC = 79;
    
    // Select the active input for TIM2_CCR1 
    // Write the CC1S bits (1 downto 0) in TIM2_CCMR1 register
    // This selects input TI1 - TIM2_CH1 for input capture
    TIM2->CCMR1 |= ~TIM_CCMR1_CC1S;
    
    // Select the active polarity for the edge detector
    // Write the CC1P to '0' and the CC1NP to '0' (active on rising edge)
    TIM2->CCER &= ~TIM_CCER_CC1P;  
    TIM2->CCER &= ~TIM_CCER_CC1NP;
    
    // Select the active input for TIM2_CCR2 
    // Write the CC2S bits to 10 in the TIM2_CCMR1 register
    TIM2->CCMR1 |= TIM_CCMR1_CC2S;
    
    // Select the active polarity for edge detection 
    // Write the CC2P to '1' and the CC2NP to '0 (active on the falling edge)
    TIM2->CCER |= TIM_CCER_CC2P;
    TIM2->CCER &= ~TIM_CCER_CC2NP;
    
    // Select the valid trigger input 
    // Write the TS bits to 100 in the TIM2_SMCR register to trigger of the TI1 edge detector (rising edge of PA0)
    TIM2->SMCR |= TIM_SMCR_ETF_2;
    
    // Configure for slave mode controller in reset mode when trigger condition occurrs -> SMS bits to 100
    TIM2->SMCR |= TIM_SMCR_SMS_2;
    
    // Enable the captures
    // Write CC1E and CC2E to '1' in TIM2_CCER
    TIM2->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);
    
    // Enable interrupts for input capture compare 2
    TIM2->DIER |= TIM_DIER_CC2IE;
    
}

