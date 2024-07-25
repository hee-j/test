#ifndef __TEST
#define __TEST
#include "mbed.h"
#include "settings.h"

#define TEST_SD 0
#define TEST_CAM 1
#define TEST_GPS 2
#define TEST_BMP 3
#define TEST_IMU 4
#define TEST_RADIO 5
#define TEST_ADC 6
#define TEST_BT 7


char set_subsystem_status(char subsystem, char status, bool printresult = false);
int get_subsystem_status(char subsystem);
int system_has_errors(void);
void GET_SYSTEM_STATUS(void);

#endif