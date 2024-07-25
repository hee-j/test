#ifndef __K_GPS_H_
#define __K_GPS_H_

#include "mbed.h"
#include "GPS/GPS.h"
#include "hw_config.h"
#include "flags.h"
#include "logging.h"

// Base & HKP

void GPS_init       (void);
void GPS_read_all   (void);

// Command interface

void GPS_TIME       (char* _params);
void GPS_LOCATION   (char* _params);
void GPS_VELOCITY   (char* _params);
void GPS_ALTITUDE   (char* _params);
void GPS_FIX        (char* _params);
void GPS_ALL        (char *_params);
void GPS_DATE       (char *_params);

#endif