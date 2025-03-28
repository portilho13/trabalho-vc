cmake_minimum_required(VERSION 3.10)
project(projeto-vc)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set OpenCV directory path
set(OpenCV_DIR "/opt/homebrew/opt/opencv") # Adjust the path as necessary

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link OpenCV libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
