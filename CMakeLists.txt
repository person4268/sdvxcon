# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

if(EXISTS ${USERHOME}/.pico-sdk)
# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)

include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
endif()
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(sdvxcon C CXX ASM)

set(FAMILY rp2040)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(sdvxcon
  src/sdvxcon.cpp
  src/usb_descriptors.c
)

pico_set_program_name(sdvxcon "sdvxcon")
pico_set_program_version(sdvxcon "0.1")

# Generate PIO header
pico_generate_pio_header(sdvxcon ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)
pico_generate_pio_header(sdvxcon ${CMAKE_CURRENT_LIST_DIR}/src/quadrature.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(sdvxcon 1)
pico_enable_stdio_usb(sdvxcon 0)

target_compile_options(sdvxcon PUBLIC -Wno-narrowing)

target_compile_definitions(sdvxcon PUBLIC PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE=0 PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_MS_OS_20_DESCRIPTOR=0)

# Add the standard include files to the build
target_include_directories(sdvxcon PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/src
)

# Add any user requested libraries
target_link_libraries(sdvxcon
        pico_stdlib pico_malloc pico_mem_ops pico_unique_id
        tinyusb_device tinyusb_board
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        cmsis_core
)

pico_add_extra_outputs(sdvxcon)