#pragma once
#include <stdio.h>

#define LOG_ON 1

#if LOG_ON == 1
#define LOG_I(format, ...) printf(format, ##__VA_ARGS__)
#elif
#define LOG_I(format, ...)
#endif
