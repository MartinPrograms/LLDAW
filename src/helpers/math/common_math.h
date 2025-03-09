#if defined(__x86_64__)
#include <cpuid.h>

static inline bool has_avx2() {
    unsigned int eax, ebx, ecx, edx;
    eax = 0x7;
    ecx = 0x0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(ecx & bit_OSXSAVE)) return false;
    if (!(ecx & bit_AVX)) return false;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & bit_AVX2) != 0;
}
#endif

#if defined(__APPLE__) && defined(__arm64__)
static inline bool has_neon() {
    // Do i even need to check? Arm64 apple devices *should* have neon
    return true;
}
#endif