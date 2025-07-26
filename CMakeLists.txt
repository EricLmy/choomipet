# The following lines of boilerplate have to be in your project's
# CMakeLists.txt file.
cmake_minimum_required(VERSION 3.16)

# Set the target chip to ESP32-C6
set(IDF_TARGET esp32c6)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(choomi_pet)