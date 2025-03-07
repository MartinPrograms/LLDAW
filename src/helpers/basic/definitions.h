// definitions.h, because C is stupid and differs per platform :(

#ifndef __unused
  #if defined(__GNUC__) || defined(__clang__)
    #define __unused __attribute__((unused))
  #elif defined(_MSC_VER)
    #define __unused
  #else
    #define __unused
  #endif
#endif
