/**
 * @file stdlib.h
 * @brief Freestanding stdlib.h stub for TernaryBit OS kernel
 *
 * This header redirects standard library functions to kernel equivalents.
 * Allows code written for hosted environments to compile in freestanding mode.
 * For HOST_BUILD (unit tests), uses the system stdlib.h instead.
 */

#ifndef _TBOS_STDLIB_H
#define _TBOS_STDLIB_H

#ifdef HOST_BUILD
/* Use system stdlib.h for host-based unit tests */
#include_next <stdlib.h>
#else
/* Kernel build: use TBOS libc stubs */
#include "tbos/libc.h"

/* Additional stdlib functions that might be needed */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* atoi/atol are provided by tbos/libc.h if needed */
#endif

#endif /* _TBOS_STDLIB_H */
