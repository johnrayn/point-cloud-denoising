cmake_minimum_required(VERSION 2.6)

project(viewer)

set (EXECUTABLE_NAME "main")
set (SOURCE_DIR "src")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Compilation flags
add_definitions (-std=c++11 -Wall)

# Qt4
# For automoc to work headers and source files must be in the same directory
set (CMAKE_AUTOMOC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

# CGAL
find_package(CGAL REQUIRED)
include (${CGAL_USE_FILE})

# Boost
find_package(Boost REQUIRED)

# Eigen3
find_package (Eigen3 REQUIRED)

# Include directories
include_directories(${INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR})

# Source files
FILE(
  GLOB_RECURSE
  ${EXECUTABLE_NAME}_SOURCES
  ${SOURCE_DIR}/**.cpp
  )

# Header files
FILE(
  GLOB_RECURSE
  ${EXECUTABLE_NAME}_HEADERS
  ${SOURCE_DIR}/**.hpp
  )

# Executable
ADD_EXECUTABLE(${EXECUTABLE_NAME}
  ${${EXECUTABLE_NAME}_SOURCES}
  ${${EXECUTABLE_NAME}_HEADERS}
  )

# Libraries
target_link_libraries(${EXECUTABLE_NAME}
    ${QT_LIBRARIES})

