#ifndef __K_LOOP_H_
#define __K_LOOP_H_

#include "mbed.h"

void manage_queues();
int log_data_to_file(char *str);

void setup_queues();

void cleanup();

void load_settings();

void broadcast_beacon();

void check_flags();

#endif