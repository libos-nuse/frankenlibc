#if defined(__x86_64__)
#include "x86_64/syscall_arch.h"
#elif defined(__i386__)
#include "i386/syscall_arch.h"
#elif defined(__ARMEL__) || defined(__ARMEB__)
#include "arm/syscall_arch.h"
#elif defined(__AARCH64EL__) || defined (__AARCH64EB__)
#include "aarch64/syscall_arch.h"
#elif defined(__PPC64__)
#include "ppc64/syscall_arch.h"
#elif defined(__PPC__)
#include "ppc/syscall_arch.h"
#elif defined(__MIPSEL__) || defined(__MIPSEB__)
#include "mips/syscall_arch.h"
#else
#error "Unknown architecture"
#endif
