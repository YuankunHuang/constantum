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
  $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
)

# Tests (CTest, dependency-free for V0).
enable_testing()
# NOTE: rebuilding from scratch — no test targets yet. Add them back here
# as each piece (rng, checksum, sim concept, wanderer, determinism test)
# is written and understood, not before.
