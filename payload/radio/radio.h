#ifndef __K_RADIO_
#define __K_RADIO_

#include "mbed.h"

// Base
void    radio_reset     (void);
char    radio_init      (void);

// RX/TX
void    radio_tx        (char* buf);

// TODO: give it a better name
void    check_tx();

// Housekeeping
int radio_get_RSSI      ();
uint8_t radio_get_temp  ();

// Command interface
void TEMP_RF            ();
void RSSI_RF            ();
void RF_ALL             ();

#endif