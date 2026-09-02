cmake_minimum_required(VERSION 3.20)
project(constantum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Ensure output: compile_commands.json
# That's it. I don't want to slow down compilation.
# We'll leave the check for clangd & CI.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Header-only core library (the platform).
add_library(constantum INTERFACE)
target_include_directories(constantum INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(constantum INTERFACE cxx_std_20)
target_compile_options(constantum INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>
  $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/Zc:preprocessor>
)

# Tests (CTest, dependency-free for V0).
enable_testing()
add_executable(test_rng tests/test_rng.cpp)
target_link_libraries(test_rng PRIVATE constantum)
add_test(NAME rng COMMAND test_rng)

add_executable(test_fixed tests/test_fixed.cpp)
target_link_libraries(test_fixed PRIVATE constantum)
add_test(NAME fixed COMMAND test_fixed)

add_executable(test_checksum tests/test_checksum.cpp)
target_link_libraries(test_checksum PRIVATE constantum)
add_test(NAME checksum COMMAND test_checksum)