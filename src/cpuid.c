#include "types.h"

#define CPUID_VENDOR_STRING 0x80000002
#define cpuid(in, a, b, c, d) __asm__("cpuid"                              \
                                      : "=a"(a), "=b"(b), "=c"(c), "=d"(d) \
                                      : "a"(in));

static inline void CPUID_getCpuIdVendorString(char vendor[17])
{
    // execute cpuid
    uint32 eax, ebx, ecx, edx;
    cpuid(CPUID_VENDOR_STRING, eax, ebx, ecx, edx);

    // write to string
    for (uint8 i = 0; i < 4; i++)
    {
        vendor[i] = eax >> (8 * i);
        vendor[i + 4] = ebx >> (8 * i);
        vendor[i + 8] = ecx >> (8 * i);
        vendor[i + 12] = edx >> (8 * i);
    }

    // append null
    vendor[16] = '\0';
}
