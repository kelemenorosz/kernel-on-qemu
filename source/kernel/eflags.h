#ifndef EFLAGS_H
#define EFLAGS_H

#define EFLAGS_CF_SHIFT 0
#define EFLAGS_RESERVED0_SHIFT 1
#define EFLAGS_PF_SHIFT 2
#define EFLAGS_RESERVED1_SHIFT 3
#define EFLAGS_AF_SHIFT 4
#define EFLAGS_RESERVED2_SHIFT 5
#define EFLAGS_ZF_SHIFT 6
#define EFLAGS_SF_SHIFT 7
#define EFLAGS_TF_SHIFT 8
#define EFLAGS_IF_SHIFT 9
#define EFLAGS_DF_SHIFT 10
#define EFLAGS_OF_SHIFT 11
#define EFLAGS_IOPL_SHIFT 12
#define EFLAGS_NT_SHIFT 14
#define EFLAGS_RESERVED3_SHIFT 15
#define EFLAGS_RF_SHIFT 16
#define EFLAGS_VM_SHIFT 17
#define EFLAGS_AC_SHIFT 18
#define EFLAGS_VIF_SHIFT 19
#define EFLAGS_VIP_SHIFT 20
#define EFLAGS_ID_SHIFT 21

extern uint32_t get_eflags();

#endif /* EFLAGS_H */