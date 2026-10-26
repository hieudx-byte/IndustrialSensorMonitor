add_executable(SensorManagerTests
    SensorManagerTests.cpp
    ${CMAKE_SOURCE_DIR}/src/Sensor.cpp
    ${CMAKE_SOURCE_DIR}/src/SensorManager.cpp
    ${CMAKE_SOURCE_DIR}/src/FileManager.cpp
    ${CMAKE_SOURCE_DIR}/src/AlertManager.cpp
    ${CMAKE_SOURCE_DIR}/src/SimulationEngine.cpp
)

target_include_directories(SensorManagerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME SensorSystemTestSuite COMMAND SensorManagerTests)
