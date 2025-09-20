; Philosophical Constants and Messages
; ==================================

; === States of Being ===
STATE_MOUNA    equ 0   ; Silence/Stillness
STATE_JNANA    equ 1   ; Knowledge/Understanding
STATE_VIPASSANA equ 2  ; Insight/Clarity

; === Core Principles ===
DHARMA_ALIGNED  equ 0x0D0A0D0A  ; Carriage Return + New Line
KARMA_BALANCED  equ 0x00000000  ; Zero state - perfect balance

; === Message Categories ===
%define MSG_STATE   1
%define MSG_DEBUG   2
%define MSG_ERROR   3
%define MSG_SUCCESS 4

; === System Messages ===
struc sysmsg
    .type       resb 1
    .text       resb 64
endstruc

; === Message Macros ===
%macro dharma_msg 2+
    db %1, ' ', 0x7C, ' ', %2, 0x0D, 0x0A, 0
%endmacro

; === Standard Messages ===
section .data

; System States
msg_system_init:     dharma_msg MSG_STATE, 'Seeking Dharma...'
msg_system_ready:    dharma_msg MSG_STATE, 'Dharma Aligned'
msg_system_halt:     dharma_msg MSG_ERROR, 'Karma Imbalance - System Halted'

; Debug Messages
debug_entering:      dharma_msg MSG_DEBUG, 'Entering %s state...'
debug_exiting:       dharma_msg MSG_DEBUG, 'Transitioning from %s...'

; Success Messages
success_complete:    dharma_msg MSG_SUCCESS, 'Sadhana Complete'
success_balanced:    dharma_msg MSG_SUCCESS, 'Karma Balanced'

; Error Messages
error_dharma:        dharma_msg MSG_ERROR, 'Dharma Disrupted'
error_attachment:    dharma_msg MSG_ERROR, 'Attachment Detected - Release and Retry'
