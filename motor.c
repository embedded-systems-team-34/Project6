/******************************************************************************
* FILENAME : motor.c          
*
* DESCRIPTION : 
*     Code for configuring motor 1 and 0 and also for setting position. 
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/

#include "pwm.h"
#include "motor.h"

#define NUM_MOTOR_POSITIONS (6)

// Motor duty cycle has resolution of 1 us
#define MOTOR_POSITON_ZERO (490)
#define MOTOR_POSITON_ONE (719)
#define MOTOR_POSITON_TWO (1039)
#define MOTOR_POSITON_THREE (1359)
#define MOTOR_POSITON_FOUR (1679)
#define MOTOR_POSITON_FIVE (1969)

// delay for the motor to change one position in increments of 100 ms
#define ONE_POSITION_DELAY (2)

// Array of positions for two motors
// motor_position[0] is position of motor 1
// motor_position[1] is position of motor 2
unsigned int motor_position[2];

unsigned int MOTOR_POS_ARR[6] = { 
    MOTOR_POSITON_ZERO,
    MOTOR_POSITON_ONE,
    MOTOR_POSITON_TWO,
    MOTOR_POSITON_THREE,
    MOTOR_POSITON_FOUR,
    MOTOR_POSITON_FIVE
};

// Move both motors to a known position after a reset
// return - the duration of delay to allow motor to reach known position
unsigned int motorInit() {
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
    
    pwmInit();
    
    // Command a known position from motors on startup
    set_motor_position(0,0);
    set_motor_position(1,0);
    
    // Set the current position of the motors
    motor_position[0] = 0;
    motor_position[1] = 0;
    
    // We don't know where the motor is, so assume worst case scenario and return possible maximum delay 
    return ONE_POSITION_DELAY * (NUM_MOTOR_POSITIONS);
}

// return - the duration of delay to allow motor to reach requested position
// Value returned is a count of 100 ms to wait e.g. 2 corresponds to 200 ms, 3 to 300 ms etc.
unsigned int set_motor_position( unsigned int channel_num, unsigned int commanded_position) {
    unsigned int position_delta;
    
    if (commanded_position >= motor_position[channel_num]) {
        position_delta = commanded_position - motor_position[channel_num];
    } else {
        position_delta = motor_position[channel_num] - commanded_position;
    }
    
    // Write new Duty cycle value for channel 1
    setPWMDuty( channel_num, MOTOR_POS_ARR[commanded_position]);
    motor_position[channel_num] = commanded_position;
    return position_delta * ONE_POSITION_DELAY;
}

// channel_num - channel to move 
// direction - +1 indicates move right, -1 indicates move left
// return - returns delay needed by motor to reach the position
unsigned int incrementalMotorMove( unsigned int channel_num, int direction) {
    
    unsigned int delay = 0;
    
    // Handle channel 0
    if (channel_num == 0) {
        // Move right iff not at full right position
        if ((direction == 1) && (motor_position[0] != 5)) {
            delay = set_motor_position(0, motor_position[0] + 1);
        } 
        // move left iff not at full left position
        else if ((direction == -1) && (motor_position[0] != 0)) {
            delay = set_motor_position(0, motor_position[0] - 1);
        }
    }
    // Handle channel 2
    else {
        // Move right iff not at full right position
        if ((direction == 1) && (motor_position[1] != 5)) {
            delay = set_motor_position(1, motor_position[1] + 1);
        } 
        // move left iff not at full left position
        else if ((direction == -1) && (motor_position[1] != 0)) {
            delay = set_motor_position(1, motor_position[1] - 1);
        }
    }
    return delay;
}

// Return the current motor position given the motor channel
unsigned int get_motor_position( unsigned int channel_num ) {
    return motor_position[channel_num];
}
