/**
 * @file time.h
 * @brief Freestanding time.h stub for TernaryBit OS kernel
 *
 * Minimal time definitions for kernel code.
 */

#ifndef _TBOS_TIME_H
#define _TBOS_TIME_H

#include <stdint.h>

typedef int64_t time_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

/* Stub implementations - kernel should provide real ones if needed */
static inline time_t time(time_t *tloc) {
    (void)tloc;
    return 0;
}

#endif /* _TBOS_TIME_H */
