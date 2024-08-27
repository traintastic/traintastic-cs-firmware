cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(traintastic-cs C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(traintastic-cs
  src/main.cpp
  src/traintasticcs/input.cpp
  src/traintasticcs/traintasticcs.cpp
  src/xpressnet/xpressnet.cpp
  src/s88/s88.cpp
)

pico_generate_pio_header(traintastic-cs
  ${CMAKE_CURRENT_LIST_DIR}/src/xpressnet/xpressnet.pio
)
pico_generate_pio_header(traintastic-cs ${CMAKE_CURRENT_LIST_DIR}/src/s88/s88.pio)

# pull in common dependencies
target_link_libraries(traintastic-cs
  pico_stdlib
  hardware_pio
)

# create map/bin/hex file etc.
pico_add_extra_outputs(traintastic-cs)
