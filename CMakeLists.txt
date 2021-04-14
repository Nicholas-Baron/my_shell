cmake_minimum_required(VERSION 3.0)
project(shell
    LANGUAGES CXX
    )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_compile_options(
    -Wall -Wextra -Wmissing-declarations
    )

add_subdirectory(src)
