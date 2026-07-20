#ifndef TernaryBitOS_TBOSBridge_h
#define TernaryBitOS_TBOSBridge_h

/* Exposes the real, already-shipped TBOS lossless codec
 * (src/core/compression/pxfs_lossless.c) to Swift - the same functions
 * src/shell/tbos_tier_shell.c's pxcompress/pxdecompress commands call.
 * No Swift reimplementation: this app links and calls the actual C code. */
#import "pxfs_lossless.h"

#endif
