#pragma once

#include <cstdio>
#include <cstdlib>

#define CHECK(cond, fmt, ...) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "CHECK failed: %s\n at %s:%d\n  " fmt "\n", \
                 #cond, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
            std::exit(1); \
        } \
    } while (0)