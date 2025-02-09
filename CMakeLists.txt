# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(firmware-embarcatech C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona o subdiretório de bibliotecas externas
# add_subdirectory(libs/pico-ssd1306)

# Add executable. Default name is the project name, version 0.1
add_executable(firmware-embarcatech
    main.c
    ${CMAKE_SOURCE_DIR}/libs/my_gpio/my_gpio.c
    ${CMAKE_SOURCE_DIR}/libs/my_wifi/my_wifi.c
    ${CMAKE_SOURCE_DIR}/libs/my_i2c/my_i2c.c
    ${CMAKE_SOURCE_DIR}/libs/my_adc/my_adc.c
    ${CMAKE_SOURCE_DIR}/libs/my_std_ret/my_std_ret.c
    ${CMAKE_SOURCE_DIR}/libs/my_std_debug/my_std_debug.c
    ${CMAKE_SOURCE_DIR}/libs/my_modbus_master/my_modbus_master.c
    ${CMAKE_SOURCE_DIR}/libs/ssd1306/ssd1306.c
)

pico_set_program_name(firmware-embarcatech "firmware-embarcatech")
pico_set_program_version(firmware-embarcatech "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(firmware-embarcatech 0)
pico_enable_stdio_usb(firmware-embarcatech 1)

# pull in common dependencies
target_link_libraries(firmware-embarcatech 
    pico_stdlib
    hardware_adc
    hardware_i2c
)

target_include_directories(firmware-embarcatech PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/my_gpio
    ${CMAKE_SOURCE_DIR}/libs/my_wifi
    ${CMAKE_SOURCE_DIR}/libs/my_i2c
    ${CMAKE_SOURCE_DIR}/libs/my_adc
    ${CMAKE_SOURCE_DIR}/libs/my_std_ret
    ${CMAKE_SOURCE_DIR}/libs/my_std_debug
    ${CMAKE_SOURCE_DIR}/libs/my_modbus_master/
    ${CMAKE_SOURCE_DIR}/libs/lwip
    ${CMAKE_SOURCE_DIR}/libs/ssd1306/
)


# Add any user requested libraries
target_link_libraries(firmware-embarcatech pico_cyw43_arch_lwip_threadsafe_background)

# create map/bin/hex file etc.
pico_add_extra_outputs(firmware-embarcatech)

# add url via pico_set_program_url