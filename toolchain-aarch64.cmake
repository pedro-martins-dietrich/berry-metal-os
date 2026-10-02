set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

find_program(
    AARCH64_GCC
    NAMES aarch64-none-elf-gcc
    REQUIRED
)

find_program(
    AARCH64_GXX
    NAMES aarch64-none-elf-g++
    REQUIRED
)

find_program(
    AARCH64_LD
    NAMES aarch64-none-elf-ld
    REQUIRED
)

find_program(
    AARCH64_OBJCOPY
    NAMES aarch64-none-elf-objcopy
    REQUIRED
)

set(CMAKE_C_COMPILER aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER aarch64-none-elf-gcc)
set(CMAKE_LINKER aarch64-none-elf-ld)
set(CMAKE_OBJCOPY aarch64-none-elf-objcopy)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
