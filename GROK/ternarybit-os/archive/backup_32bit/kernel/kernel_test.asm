; Simple test kernel - just halt
[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Just halt - don't do anything else
    cli
    hlt
    jmp $
