#ifndef __BUZZER_H_
#define __BUZZER_H_

#include "mbed.h"
#include "hw_config.h"

#include "logging.h"
#include "morse.h"

void morse(char* morse_str);
void beep(unsigned char times);
void buzzer_init(void);
void quick_beep(void);

// Command interface
void BEEP       (char *_params);
void STOP_BEEP  (char *_params);
void MORSE      (char *_params);

void morse_thread(void);

#define ONE_UNIT    60ms
#define TWO_UNITS   120ms
#define THREE_UNITS 180ms
#define SIX_UNITS   360ms

#endif