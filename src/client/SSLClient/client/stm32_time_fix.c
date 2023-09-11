
// _gettimeofday link missing in STM32 

#if defined(ARDUINO_ARCH_STM32)

#include <sys/time.h>

__attribute__((weak)) int _gettimeofday(struct timeval *tv, void *ignore __attribute__((unused)))
{
    uint64_t t = 0;
    tv->tv_sec = t / 1000000000;
    tv->tv_usec = (t % 1000000000) / 1000;
    return 0;
}

#endif