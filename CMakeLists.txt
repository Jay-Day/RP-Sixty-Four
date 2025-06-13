cmake_minimum_required(VERSION 3.13)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(n64_controller C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable
add_executable(n64_controller
    src/main.c
    src/n64_protocol.c
    src/encoder.c
    src/buttons.c
    src/controller_pak.c
)

# Pull in common dependencies
target_link_libraries(n64_controller 
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
    hardware_timer
    hardware_gpio
    hardware_irq
    hardware_flash
    hardware_sync
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(n64_controller)

# Add PIO programs
pico_generate_pio_header(n64_controller ${CMAKE_CURRENT_LIST_DIR}/src/n64_protocol.pio) 