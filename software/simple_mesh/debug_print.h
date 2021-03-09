#ifndef DEBUG_PRINT
#ifdef DEBUG__
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif
#endif