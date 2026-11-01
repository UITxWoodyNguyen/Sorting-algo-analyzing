cmake_minimum_required(VERSION 3.16)

if (NOT DEFINED CMAKE_CXX_COMPILER)
    if (WIN32 AND EXISTS "C:/msys64/mingw64/bin/g++.exe")
        set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe" CACHE FILEPATH
            "GNU C++ compiler")
    elseif (UNIX AND EXISTS "/usr/bin/g++")
        set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE FILEPATH
            "GNU C++ compiler")
    endif()
endif()

project(SortingAlgoAnalyzing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    message(FATAL_ERROR "Hay dung g++ MinGW: C:/msys64/mingw64/bin/g++.exe")
endif()

add_library(sort_core INTERFACE)
target_include_directories(sort_core INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(sort_app src/main.cpp src/bench.cpp src/kat.cpp)
target_link_libraries(sort_app PRIVATE sort_core)

add_executable(bench src/bench.cpp src/bench_main.cpp)
target_link_libraries(bench PRIVATE sort_core)

add_executable(kat src/kat.cpp src/kat_main.cpp)
target_link_libraries(kat PRIVATE sort_core)

add_executable(generator dataset/generator.cpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sort_app PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(bench PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(kat PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(generator PRIVATE -Wall -Wextra -pedantic)
endif()
