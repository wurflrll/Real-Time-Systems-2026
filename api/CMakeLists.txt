cmake_minimum_required(VERSION 3.16)

project(RT-Systems LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(app ${SOURCES})

target_compile_options(app PRIVATE
    -Wall
    -Wextra
)

target_link_libraries(app PRIVATE
    avformat
    avcodec
    avutil
    swscale
    swresample
    ssl
    crypto
    pthread
    z
)