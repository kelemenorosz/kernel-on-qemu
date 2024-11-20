#ifndef MSR_H
#define MSR_H

uint32_t check_msr();
extern void read_msr(uint32_t ecx, uint32_t* eax, uint32_t* edx);

#endif /* MSR_H */