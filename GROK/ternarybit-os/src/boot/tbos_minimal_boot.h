/**
 * @file tbos_minimal_boot.h
 * @brief TernaryBit OS - Minimal Bootloader API
 *
 * US-1.1: Minimal Hardware Bootstrap
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_MINIMAL_BOOT_H
#define TBOS_MINIMAL_BOOT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* HARDWARE CLASSES                                                          */
/* ========================================================================= */

typedef enum {
    HW_CALCULATOR,      /* Calculator-class (2KB RAM) */
    HW_RADIO,           /* Radio computer (8KB RAM) */
    HW_MINIMAL,         /* Minimal system (16KB RAM) */
    HW_STANDARD,        /* Standard system (64KB+ RAM) */
    HW_ADVANCED         /* Advanced system (1MB+ RAM) */
} hardware_class_t;

/* ========================================================================= */
/* BOOT API                                                                  */
/* ========================================================================= */

/**
 * @brief Main boot entry point
 * @return 0 on success, negative on error
 */
int tbos_minimal_boot(void);

/**
 * @brief Detect hardware capabilities
 */
void detect_hardware(void);

/**
 * @brief Initialize consciousness state
 */
void init_consciousness(void);

/**
 * @brief Get hardware class name
 * @param class Hardware class enum
 * @return String name of hardware class
 */
const char* get_hardware_name(hardware_class_t class);

/* ========================================================================= */
/* HARDWARE-SPECIFIC I/O (To be implemented per platform)                   */
/* ========================================================================= */

#ifdef EMBEDDED_BUILD
/**
 * @brief Output single character to hardware
 * @param c Character to output
 */
extern void hw_putc(char c);

/**
 * @brief Read single character from hardware
 * @return Character read
 */
extern char hw_getc(void);

/**
 * @brief Delay for specified milliseconds
 * @param ms Milliseconds to delay
 */
extern void hw_delay_ms(uint16_t ms);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TBOS_MINIMAL_BOOT_H */
