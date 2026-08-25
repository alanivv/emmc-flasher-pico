cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_NO_CYW43_SUPPORT 1)
set(PICO_NO_CYW43_SUPPORT 1)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(emmc_fast_flasher C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(emmc_fast_flasher
    main.c
)

target_link_libraries(emmc_fast_flasher
    pico_stdlib
    hardware_pio
    hardware_clocks
    hardware_gpio
)

pico_enable_stdio_usb(emmc_fast_flasher 1)
pico_enable_stdio_uart(emmc_fast_flasher 0)

pico_add_extra_outputs(emmc_fast_flasher)