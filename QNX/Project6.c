#include <stdlib.h>
#include <stdio.h>

#include <sys/neutrino.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

#define QNX_BASE_ADDRESS (0x280)
#define PWM_PIN (1)
#define DEBUG (0)

// See input range selection table in user manual, for gain of 1 resolution is 305 uV
#define ADC_LSB_RESOLUTION (0.000305)

#define ADC_MAX (16666)         // Corresponds to 5V 305 uV/bit (resolution)
#define ADC_MIN (-16666)        // Corresponds to -5V 305 uV/bit (resolution)

#define MEASURE_CYCLE_PERIOD_NS (2000000)	// 2 ms
#define MAX_PWM_PULSE_NS (1000000)
#define MIN_PWM_PULSE_NS (1000)
#define NS_PER_US (1000)
#define FLOAT_MAX_PWM_PULSE_US (1000.0)

// This offset is necessary to account for the capacitance on PA0 which takes approximately 25 us to rise
#define PULSE_OFFSET (25000)

//Fixed A/C channel
#define ADC_CHANNEL(0x00) //Channel0 at base+2

//Represents the statuses within A/D control registers
#define ADC_START (0x80) //STARTAD at base+0
#define ADC_BUSY (0x80) //ADBUSY at base+0
#define ADC_WAIT (0x20) //ADWAIT at base+3
#define ADC_POL (0x00) //ADPOL on page 2, at base+13
#define ADC_GAIN (0x00) //ADG0 at base+3

uintptr_t port_a;
uintptr_t dir;

// Initialize the PWM hardware - get hardware permission, and create pwm thread
void pwmInit() {

	pthread_t pwm_th;

	// Get access to hardware IO
    if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1)
    {
    	printf("Failed to get I/O access permission");
    }

    // PORTA is located at QNX_BASE_ADDRESS + 8 (register 0x288)
    port_a = mmap_device_io(1, QNX_BASE_ADDRESS + 8);
    dir = mmap_device_io(1, QNX_BASE_ADDRESS + 11);
    
    // Set PortA as output
    out8(dir,0x00);
}

// Normalize the sampled adcValue between 0.0 - 1.0
// 0 corresponds to ADC_MIN -5 VDC
// 1 corresponds to ADC_MAX  5VDC
float normalizeADCVal(int adcVal) {
    // if adcVal greater than max value clip to max value
	if (adcVal > ADC_MAX) return 1.0;
    // if adcVal less than min value clip to min value
	if (adcVal < ADC_MIN) return 0.0;
    // Normalize values in between max and min between 0.0 and 1.0
	return (float)(adcVal - ADC_MIN)/(float)(ADC_MAX-ADC_MIN);
}

// Encode the normalized measured voltage into a PWM duty cycle which can be 
// transmitted to the STM32
uint32_t getPWMPulseLength(float norm_adc_val) {
	if (norm_adc_val >= 1.0) return MAX_PWM_PULSE_NS;
	if (norm_adc_val <= 0.0) return MIN_PWM_PULSE_NS;
    // Calculate pulse length in us, then scale to ns for timer 
	return (uint32_t)(norm_adc_val*FLOAT_MAX_PWM_PULSE_US) * NS_PER_US;

}

// Initalize the ADC For fixed channel, input range -10 to 10
void adcInit() {
	// Set channel range to fixed channel 0
	out8(QNX_BASE_ADDRESS + 2, ADC_CHANNEL);

	// Select the input range
	// Use a gain of 1 +10V to -10V
	out8(QNX_BASE_ADDRESS + 3, ADC_GAIN);

	// Select the Polarity
	// Set to page 2 of A/D control registers
	out8(QNX_BASE_ADDRESS + 1, 2);
	// Set bi-polar, single ended
	out8(QNX_BASE_ADDRESS+13, ADC_POL);

	// Wait for analog input circuit to settle
    //Wait until ADWAIT goes low
	while(in8(QNX_BASE_ADDRESS + 3) & ADC_WAIT);
}

int main(int argc, char *argv[]) {

	int16_t adc_lsb;
    int16_t adc_msb;
	int16_t adcVal;
	uint32_t pwm_pulse_length;
	struct timespec timer_spec;
	float norm_adc_val;

	printf("Starting ADC measurements!\n");

	pwmInit();
    adcInit();

    // main loop 
        // Convert samples
        // Calculate PWM signal from sampled value
        // Transmit PWM signal
        // This runs at 500 Hz
	while(1) {
		// Perform a conversion on the current channel
		out8(QNX_BASE_ADDRESS, ADC_START);

		// Poll the status flag for the conversion to finish
        //Wait until ADBUSY goes low
		while (in8(QNX_BASE_ADDRESS + 3) & ADC_BUSY);
        // Read the ADC reading when status indicates conversion finished
		adc_lsb = in8(QNX_BASE_ADDRESS);
		adc_msb = in8(QNX_BASE_ADDRESS+1);
		// Combine the two bytes into the adc reading
		adcVal = adc_lsb | (adc_msb << 8);

		// Normalize the measured ADC voltage between 0.0 and 1.0
		norm_adc_val = normalizeADCVal(adcVal);
		// Determine the pulse length to be transmitted based on the normalized adcValue
		pwm_pulse_length = getPWMPulseLength(norm_adc_val);

#if DEBUG
		printf("Reading: %d\n", adcVal);
		printf("Voltage: %f\n", adcVal * ADC_LSB_RESOLUTION);
		printf("Norm val %f\n", normalizeADCVal(adcVal));
		printf("PWM pulse length: %u\n", pwm_pulse_length);
#endif

		// Start the PWM signal
		timer_spec.tv_sec = 0;
        // Add a 25 us delay to the encoded pulse.  This is due to the input capacitance 
        // on PA0 of the STM32.  This pin has a 25 us rise time due to the 100 us cap 
        // on PA0.  Therefore add this offset so all pulses can be detected by STM32
		pwm_pulse_length += PULSE_OFFSET;
		timer_spec.tv_nsec = pwm_pulse_length;
        // Set the PWM signal high to start the transmission, STM32 will start measurement here
		out8(port_a, PWM_PIN);
		// Wait for length of duty cycle
		nanospin(&timer_spec);
        // Set the PWM signal low to end the transmission, STM32 will stop measurement here
		out8(port_a, 0);

		// Sleep for the remainder of the measure cycle period to sample the ADC input periodically 
		timer_spec.tv_sec = 0;
		timer_spec.tv_nsec = MEASURE_CYCLE_PERIOD_NS - pwm_pulse_length;
		nanosleep(&timer_spec, NULL);
	}
	return EXIT_SUCCESS;
}
