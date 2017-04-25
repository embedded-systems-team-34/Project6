/******************************************************************************
* FILENAME : motor.h          
*
* DESCRIPTION : 
*     Function prototypes for motor
*
* AUTHOR: 
*     Donald MacIntyre - djm4912@rit.edu
*     Madison Smith    - ms8565@rit.edu  
*
******************************************************************************/
#ifndef MOTOR_H
#define MOTOR_H 


// Function Prototypes
unsigned int motorInit(void);
unsigned int set_motor_position( unsigned int channel_num, unsigned int position);
unsigned int incrementalMotorMove( unsigned int channel_num, int direction);
unsigned int get_motor_position( unsigned int channel_num );

#endif
