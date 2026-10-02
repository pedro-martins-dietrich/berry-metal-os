.section .text.boot
.global _start
.type _start, %function

_start:
    // Use only core 0
    mrs x0, mpidr_el1
    and x0, x0, #0x3
    cbnz x0, halt

    // Setup stack
    ldr x0, =__stack_top
    mov sp, x0

    // Clear BSS
    ldr x0, =__bss_begin
    ldr x1, =__bss_end
clear_bss_loop:
    cmp x0, x1
    bge bss_cleared
    str xzr, [x0], #8
    b clear_bss_loop
bss_cleared:

    // Call the main function
    bl main
halt:
    wfe
    b halt

.size _start, . - _start
