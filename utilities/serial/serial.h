#ifndef __SERIAL_H
#define __SERIAL_H

#include "mbed.h"


void    pc_printf(const char *fmt, ...);
void    pc_putc(char chr);
void    setup_serial(void);
int     command_callback();

#endif // __SERIAL_H