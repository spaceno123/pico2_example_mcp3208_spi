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
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mcp3208_spi C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(mcp3208_spi mcp3208_spi.c )

pico_set_program_name(mcp3208_spi "mcp3208_spi")
pico_set_program_version(mcp3208_spi "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(mcp3208_spi 1)
pico_enable_stdio_usb(mcp3208_spi 0)

# Add the standard library to the build
target_link_libraries(mcp3208_spi
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(mcp3208_spi PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(mcp3208_spi 
        hardware_spi
        )

pico_add_extra_outputs(mcp3208_spi)

