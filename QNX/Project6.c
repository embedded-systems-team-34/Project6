#include <stdlib.h>
#include <stdio.h>

#include <sys/neutrino.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

#define QNX_BASE_ADDRESS (0x280)

// See input range selection table in user manual, for gain of 1 resolution is 305 uV
#define ADC_LSB_RESOLUTION (0.000305)

#define ADC_MAX (16666)         // Corresponds to 5V 305 uV/bit (resolution)
#define ADC_MIN (-16666)        // Corresponds to -5V 305 uV/bit (resolution)
#define DEBUG (0)
#define MEASURE_CYCLE_PERIOD (2000000)	// 2 ms
#define MAX_PWM_PULSE_NS (1000000)
#define NS_PER_US (1000)
#define FLOAT_MAX_PWM_PULSE_US (1000.0)
#define MIN_PWM_PULSE_NS (1000)
// This offset is necessary to account for the capacitance on PA0 which takes approximately 25 us to rise
#define PULSE_OFFSET (25000)
#define PWM_PIN (1)
#define ADC_CONVERSION_START (0x80)

uintptr_t port_a;
uintptr_t dir;

// Initalize the PWM hardware - get hardware permission, and create pwm thread
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
uint32_t getpwmPulseLength(float normAdcVal) {
	if (normAdcVal >= 1.0) return MAX_PWM_PULSE_NS;
	if (normAdcVal <= 0.0) return MIN_PWM_PULSE_NS;
    // Calculate pulse length in us, then scale to ns for timer 
	return (uint32_t)(normAdcVal*FLOAT_MAX_PWM_PULSE_US) * NS_PER_US;

}

// Initalize the ADC For fixed channel, input range -10 to 10
void adcInit() {
	// Set channel range to fixed channel 0
	out8(QNX_BASE_ADDRESS + 2, 0x00);

	// Select the input range
	// Use a gain of 1 +10V to -10V
	out8(QNX_BASE_ADDRESS + 3,0x00);

	// Select the Polarity
	// Set to page 2
	out8(QNX_BASE_ADDRESS + 1, 2);
	// Set bi-polar, single ended
	out8(QNX_BASE_ADDRESS+13, 0x00);

	// Wait for analog input circuit to settle
	while(in8(QNX_BASE_ADDRESS + 3) & 0x20);
}

int main(int argc, char *argv[]) {

	int16_t adc_lsb;
    int16_t adc_msb;
	int16_t adcVal;
	uint32_t pwmPulseLength;
	struct timespec timer_spec;
	float normADCval;

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
		out8(QNX_BASE_ADDRESS, ADC_CONVERSION_START);

		// Poll the status flag for the conversion to finish
		while (in8(QNX_BASE_ADDRESS + 3) & ADC_CONVERSION_START);
        // Read the ADC reading when status indicates conversion finished
		adc_lsb = in8(QNX_BASE_ADDRESS);
		adc_msb = in8(QNX_BASE_ADDRESS+1);
		// Combine the two bytes into the adc reading
		adcVal = adc_lsb | (adc_msb << 8);

		// Normalize the measured ADC voltage between 0.0 and 1.0
		normADCval = normalizeADCVal(adcVal);
		// Determine the pulse length to be transmitted based on the normalized adcValue
		pwmPulseLength = getpwmPulseLength(normADCval);

#if DEBUG
		printf("Reading: %d\n", adcVal);
		printf("Voltage: %f\n", adcVal * ADC_LSB_RESOLUTION);
		printf("Norm val %f\n", normalizeADCVal(adcVal));
		printf("PWM pulse length: %u\n", pwmPulseLength);
#endif

		// Start the PWM signal
		timer_spec.tv_sec = 0;
        // Add a 25 us delay to the encoded pulse.  This is due to the input capacitance 
        // on PA0 of the STM32.  This pin has a 25 us rise time due to the 100 us cap 
        // on PA0.  Therefore add this offset so all pulses can be detected by STM32
		pwmPulseLength += PULSE_OFFSET;
		timer_spec.tv_nsec = pwmPulseLength;
        // Set the PWM signal high to start the transmission, STM32 will start measurement here
		out8(port_a, PWM_PIN);
		// Wait for length of duty cycle
		nanospin(&timer_spec);
        // Set the PWM signal low to end the transmission, STM32 will stop measurement here
		out8(port_a, 0);

		// Sleep for the remainder of the measure cycle period to sample the ADC input periodically 
		timer_spec.tv_sec = 0;
		timer_spec.tv_nsec = MEASURE_CYCLE_PERIOD - pwmPulseLength;
		nanosleep(&timer_spec, NULL);
	}
	return EXIT_SUCCESS;
}
