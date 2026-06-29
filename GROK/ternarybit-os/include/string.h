/**
 * @file string.h
 * @brief Freestanding string.h stub for TernaryBit OS kernel
 *
 * This header redirects string functions to kernel equivalents.
 * For HOST_BUILD (unit tests), uses the system string.h instead.
 */

#ifndef _TBOS_STRING_H
#define _TBOS_STRING_H

#ifdef HOST_BUILD
/* Use system string.h for host-based unit tests */
#include_next <string.h>
#else
/* Kernel build: use TBOS libc stubs */
#include "tbos/libc.h"

/* String functions are provided by tbos/libc.h:
 * - memset, memcpy, memmove, memcmp
 * - strlen, strcpy, strncpy, strcmp, strncmp
 * - strcat, strchr, strstr
 */
#endif

#endif /* _TBOS_STRING_H */
