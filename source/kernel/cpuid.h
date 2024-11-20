#ifndef CPUID_H
#define CPUID_H

extern uint32_t check_cpuid();  
extern void get_cpuid(uint32_t value, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);

#endif /* CPUID_H */