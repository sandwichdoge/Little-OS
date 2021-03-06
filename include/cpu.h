#ifndef INCLUDE_CPU_H
#define INCLUDE_CPU_H
#include "drivers/cpuid.h"
#include "stddef.h"

#define CPU_EFLAGS_IF (1 << 9)

size_t get_flags_reg();
void cpu_relax();

#endif
