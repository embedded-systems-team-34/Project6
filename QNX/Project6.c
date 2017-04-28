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

// Normalize the sampled adcValue between 0.0 - 1.0
// 0 corresponds to ADC_MIN 
// 1 corresponds to ADC_MAX
float normalizeADCVal(int adcVal) {
	return (float)(adcVal - ADC_MIN)/(float)(ADC_MAX-ADC_MIN);
}

int main(int argc, char *argv[]) {

	int16_t adc_lsb
    int16_t adc_msb;
	int16_t adcVal;

	printf("Starting ADC measurements!\n");

	// Get access to hardware IO
    if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1)
    {
        printf("Failed to get I/O access permission");
    }

    // PORTA is located at QNX_BASE_ADDRESS + 8 (register 0x288)
    //   port_a = mmap_device_io(1, QNX_BASE_ADDRESS + 8);
    //   dir = mmap_device_io(1, QNX_BASE_ADDRESS + 11);

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

	// Perform a conversion on the current channel
	out8(QNX_BASE_ADDRESS, 0x80);

	// Poll the status flag for the conversion to finish
	while (in8(QNX_BASE_ADDRESS + 3) & 0x80);
	adc_lsb = in8(QNX_BASE_ADDRESS);
	adc_msb = in8(QNX_BASE_ADDRESS+1);
	printf("ADC reading 0x%x\n", lsb);
	printf("ADC reading 0x%x\n", msb);
    // Combine the two results into the adc reading
	adcVal = adc_lsb | (adc_msb << 8);
	printf("Reading: %d\n", adcVal);
	printf("Voltage: %f\n", adcVal * ADC_LSB_RESOLUTION);
	printf("Norm val %f\n", normalizeADCVal(adcVal));

	return EXIT_SUCCESS;
}
