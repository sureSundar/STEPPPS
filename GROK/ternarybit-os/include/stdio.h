/**
 * @file stdio.h
 * @brief Freestanding stdio.h stub for TernaryBit OS kernel
 *
 * This header redirects stdio functions to kernel equivalents.
 * For HOST_BUILD (unit tests), uses the system stdio.h instead.
 */

#ifndef _TBOS_STDIO_H
#define _TBOS_STDIO_H

#if defined(HOST_BUILD) || defined(TBOS_HOSTED)
/* Use system stdio for host-based builds */
#include_next <stdio.h>
#else
/* Kernel build: use TBOS stdio stubs */
#include "tbos/stdio.h"
#endif

#endif /* _TBOS_STDIO_H */
