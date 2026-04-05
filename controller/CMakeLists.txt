set(PICO_BOARD pico2 CACHE STRING "Board Type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(controller C CXX ASM)
pico_sdk_init()

# Add compilation subdirectory
add_subdirectory(lib/Config)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(lib/LCD)

# Add header file directory
include_directories(./lib/Config)
include_directories(./lib/Fonts)
include_directories(./lib/GUI)
include_directories(./lib/LCD)

add_executable(controller main.c)

target_sources( controller PRIVATE
    state.c
    display.c
    tasks.c
    task_misc.c
    task_serial.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(controller 1)
pico_enable_stdio_uart(controller 1)

pico_add_extra_outputs(controller)

target_link_libraries(controller PRIVATE
    Config
    Fonts
    GUI
    LCD
    pico_stdlib
    hardware_spi
)
