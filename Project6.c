#include <stdlib.h>
#include <stdio.h>

#include <sys/neutrino.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

#define QNX_BASE_ADDRESS (0x280)

int main(int argc, char *argv[]) {

	unsigned int lsb, msb;

	printf("Welcome to the hello QNX Momentics IDE\n");


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

	printf("Fixed channel\n");

	// Select the input range
	// Use a gain of 1 +10V to -10V
	out8(QNX_BASE_ADDRESS + 3,0x00);

	// Select the Polarity
	// Set to page 2
	out8(QNX_BASE_ADDRESS + 1, 2);
	// Set bi-polar, single ended
	out8(QNX_BASE_ADDRESS+13, 0x00);

	printf("Polling anlog input %u\n");

	// Wait for analog input circuit to settle
	while(in8(QNX_BASE_ADDRESS + 3) & 0x20);

	// Perform a conversion on the current channel
	out8(QNX_BASE_ADDRESS, 0x80);

	printf("Polling convesion finish %u\n");

	// Poll the status flag for the conversion to finish
	while (in8(QNX_BASE_ADDRESS + 3) & 0x80);
	lsb = in8(QNX_BASE_ADDRESS);
	msb = in8(QNX_BASE_ADDRESS+1);
	printf("ADC reading 0x%x\n", lsb);
	printf("ADC reading 0x%x\n", msb);





	return EXIT_SUCCESS;
}
