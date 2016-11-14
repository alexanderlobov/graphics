cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(obfuscate)

find_package(PCL 1.3 REQUIRED COMPONENTS io surface)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_definitions(-O2 -Wall)

# find_package(Boost COMPONENTS system REQUIRED)
# include_directories(${Boost_INCLUDE_DIRS})

add_executable(obfuscate
    obfuscate.cpp
)

target_link_libraries(obfuscate ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} ${PCL_SURFACE_LIBRARIES})
