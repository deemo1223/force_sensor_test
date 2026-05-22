cmake_minimum_required(VERSION 3.16)

project(force_sensor_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(force_sensor_test
  src/main.cpp
  src/force_sensor/ForceSensorReader.cpp
  src/force_sensor/PosixSerialPort.cpp
  src/sri/sriCommParser.cpp
  src/sri/sriCommCircularBuffer.cpp
  src/sri/sriCommATParser.cpp
  src/sri/sriCommM8218Parser.cpp
)

target_include_directories(force_sensor_test PRIVATE
  ../include/ct_rl_pkg
  src
  src/sri
)

target_link_libraries(force_sensor_test PRIVATE pthread)
