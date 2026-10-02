ENTRY(_start)

SECTIONS
{
    . = 0x80000;

    .text : ALIGN(16)
    {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata : ALIGN(16)
    {
        *(.rodata*)
    }

    .data : ALIGN(16)
    {
        *(.data*)
    }

    .bss : ALIGN(16)
    {
        __bss_begin = .;

        *(.bss*)
        *(COMMON)

        . = ALIGN(16);

        __bss_end = .;
    }

    . = ALIGN(16);

    __stack_bottom = .;
    . += 64K;
    __stack_top = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
