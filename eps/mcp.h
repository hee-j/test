#ifndef __ADC_H_
#define __ADC_H_

#include "mbed.h"

int adc_init(void);
void adc_measure();

// Command interface
void EPS_MEASURE_ALL();
void SP_C(char* params);
void SP_V(char* params);
void BATT_V(char* _params);
void GET_CHARGING_STATUS(void);







#endif