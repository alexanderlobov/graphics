cmake_minimum_required(VERSION 2.8)

project(obfuscate-with-texture)

if (WIN32)
    SET(BOOST_ROOT "C:\\Dev\\Boost")
    SET(Boost_USE_STATIC_LIBS TRUE)
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if (UNIX)
    add_definitions(-Wall -O2)
endif()

add_executable(obfuscate-with-texture
    obfuscate-with-texture.cpp
)

# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

target_link_libraries(obfuscate-with-texture
    ${Boost_LIBRARIES}
)
