cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_photoframe C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# FatFS (incluida en el repo como submodulo o carpeta)
add_subdirectory(fatfs/FatFs_SPI)

add_executable(pico_photoframe
    src/main.c
    src/st7789.c
    src/slideshow.c
    src/sd_card.c
    src/hw_config.c
)

target_include_directories(pico_photoframe PRIVATE
    include
    fatfs/FatFs_SPI/ff15/source
    fatfs/FatFs_SPI/sd_driver
    fatfs/FatFs_SPI/include
)

target_link_libraries(pico_photoframe
    pico_stdlib
    hardware_spi
    hardware_dma
    hardware_gpio
    FatFs_SPI
)

pico_add_extra_outputs(pico_photoframe)

# UART stdio para debug
pico_enable_stdio_uart(pico_photoframe 0)
pico_enable_stdio_usb(pico_photoframe 1)
