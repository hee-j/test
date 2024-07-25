#pragma once
#ifndef __LOGGING_H_
#define __LOGGING_H_

#include "mbed.h"

// Debug function with parameters
void _debug     (const char *fmt, ...);
void _error     (const char *fmt, ...);
void _warning   (const char *fmt, ...);

// Print ("ok, "fail", etc.) with no leading [i] signs
void _ack       (const char *fmt, ...);

#endif // __LOGGING_H_