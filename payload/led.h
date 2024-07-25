#ifndef __LED_H_
#define __LED_H_

#include <string.h>
#include "logging.h"
#include "../utilities/m_util.h"

// Command interface
void LED_LIT    (char* _params);
void LED_BLINK  (char* _params);
void LED_START  (char* _params);
void LED_STOP   (char* _params);


void leds_init();
void led_thread();

#endif