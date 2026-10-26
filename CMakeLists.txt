cmake_minimum_required(VERSION 3.20)

project(IndustrialSensorMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GCC / Clang / MSVC compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Core executable target
add_executable(IndustrialSensorMonitor
    src/main.cpp
    src/Sensor.cpp
    src/SensorManager.cpp
    src/FileManager.cpp
    src/AlertManager.cpp
    src/SimulationEngine.cpp
)

target_include_directories(IndustrialSensorMonitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CTest automated testing integration
enable_testing()
add_test(NAME IndustrialSensorMonitor_AllTests COMMAND IndustrialSensorMonitor --test)
