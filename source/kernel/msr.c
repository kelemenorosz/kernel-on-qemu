#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "cpuid.h"

uint32_t check_msr() {

	uint32_t eax = 0x0; 
	uint32_t ebx = 0x0; 
	uint32_t ecx = 0x0; 
	uint32_t edx = 0x0; 

	get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

	if (edx & 0x20) return 0x0;

	return 0x1;

}