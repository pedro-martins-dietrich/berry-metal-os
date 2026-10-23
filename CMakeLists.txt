cmake_minimum_required(VERSION 4.4.0)

project("RaspberryPi3B" LANGUAGES CXX ASM)

set(KERNEL "kernel.elf")
set(KERNEL_IMAGE "kernel8.img")
set(LINKER_FILE "${CMAKE_SOURCE_DIR}/linker.ld")

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_TRUE_BUILD_DIR "${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES "src/**.cpp")
file(GLOB_RECURSE ASM_FILES "src/**.s")

add_executable(${KERNEL} ${ASM_FILES} ${SRC_FILES})

target_compile_options(
    ${KERNEL} PRIVATE
    -mcpu=cortex-a53
    -ffreestanding
    -fno-builtin
    -fno-rtti
    -fno-stack-protector
    -Wall
    -Wextra
    -O2
)

target_link_options(
    ${KERNEL} PRIVATE
    -T ${LINKER_FILE}
    -nostdlib
    -nodefaultlibs
    -nostartfiles
    "-Wl,--gc-sections"
    "-Wl,--no-warn-rwx-segments"
)

set_target_properties(
    ${KERNEL} PROPERTIES
    LINK_DEPENDS ${LINKER_FILE}
)

add_custom_command(
    TARGET ${KERNEL}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${KERNEL}> "${CMAKE_BINARY_DIR}/${KERNEL_IMAGE}"
    COMMENT "Creating bare-metal kernel image."
)

add_custom_command(
    TARGET ${KERNEL}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${KERNEL_IMAGE}" "${CMAKE_SOURCE_DIR}/SD Card/${KERNEL_IMAGE}"
    COMMENT "Copied kernel image to the SD Card folder."
)
