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
#include "pwm.h"

// Maximum  and Minimum PWM On Time In us
#define MAX_PWMTIME (1000)
#define MIN_PWMTIME (0)
// PWM Offset for the pin's 25us rise time
#define PWM_OFFSET (25)

void TIM2_IRQHandler(void) {
    uint16_t which_interrupt = TIM2->SR;
	uint32_t pwm_duty_time;
    
    // Input channel 2 capture interrupt, measure the duty cycle 
    if ((which_interrupt & TIM_SR_CC2IF) == TIM_SR_CC2IF) {
        // TIM2->CCR2 will contain pulse duration in us since CCR2 launched on rising edge
        // and input fires on the falling edge
        // Read duty cycle of the received pulse in us
		pwm_duty_time = TIM2->CCR2;
		// Subtract of 25 us to account for the 25 us offset added on QNX which is necessary as PA0 has approximately 
        // a 25 us rise time due to the capacitance on the input of all pins on PortA
        pwm_duty_time -= PWM_OFFSET;
        // Normalize the received PWM signal and update the PWM module with the latest pwm duty time
		setNormalizedPWMDuty(normalizePWMTime(pwm_duty_time));
    }
}

// Return a value between 0 and 1 indicating the duration of the PWM on time
// 0.0 corresponds to a MIN_PWMTIME of 0 us
// 1.0 corresponds to a MAX_PWMTIME of 1000 us
float normalizePWMTime(uint32_t duty_time) {
    if (duty_time >= MAX_PWMTIME) {
        return 1.0;
    } else {
        return (float)(duty_time-MIN_PWMTIME) / (float)(MAX_PWMTIME - MIN_PWMTIME);
    }
}

// Configure for PWM Input Mode to measure duty cycle
// See 27.3.6 - PWM Input Mode of the reference manual
void timer2PWMInputModeInit() {
    
    // Enable clk to PortA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    // Configure GPIO Pin A0 for alternate function AF1 such that it is routed
    // to TIM2_CH1
    
    // Enable the clock to GPIO Ports A    
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    // Set PA0 to be alternate function
    GPIOA->MODER &= ~GPIO_MODER_MODER0;        // Clear moder register mode0[1:0]
    GPIOA->MODER |= GPIO_MODER_MODER0_1;    // Set alternate function mode 10
    
    // Set Alternate function lower register to AF1 so that A1 is set connected to TIM2_CH1
    GPIOA->AFR[0] = 0x1;	
	
    // Enable clock of timer 2
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    
    // Only allow an update event (UIF) when an overflow occurs 
    TIM2->CR1 |= TIM_CR1_URS;
        
    // Set Prescaler
    // 80 MHz / 80 = 1 MHz -> 1 us
    TIM2->PSC = 79;
	  TIM2->EGR |= 1;
    
    // Select the active input for TIM2_CCR1 
    // Write the CC1S bits (1 downto 0) in TIM2_CCMR1 register
    // This selects input TI1 - TIM2_CH1 for input capture
    TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
    
    // Select the active polarity for the edge detector
    // Write the CC1P to '0' and the CC1NP to '0' (active on rising edge)
    TIM2->CCER &= ~TIM_CCER_CC1P;  
    TIM2->CCER &= ~TIM_CCER_CC1NP;
    
    // Select the active input for TIM2_CCR2 
    // Write the CC2S bits to 10 in the TIM2_CCMR1 register
    TIM2->CCMR1 |= TIM_CCMR1_CC2S_1;
    
    // Select the active polarity for edge detection 
    // Write the CC2P to '1' and the CC2NP to '0 (active on the falling edge)
    TIM2->CCER |= TIM_CCER_CC2P;
    TIM2->CCER &= ~TIM_CCER_CC2NP;
    
    // Select the valid trigger input 
    // Write the TS bits to 100 in the TIM2_SMCR register to trigger of the TI1 edge detector (rising edge of PA0)
    TIM2->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;
    
    // Configure for slave mode controller in reset mode when trigger condition occurrs -> SMS bits to 100
    TIM2->SMCR |= TIM_SMCR_SMS_2;
    
    // Enable the captures
    // Write CC1E and CC2E to '1' in TIM2_CCER
    TIM2->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);
    
    // Enable interrupts for input capture compare 2
    TIM2->DIER |= TIM_DIER_CC2IE;
	TIM2->CR1 |= TIM_CR1_CEN;
    
}
