#ifndef __EXTERNAL_PAYLOAD_H
#define __EXTERNAL_PAYLOAD_H

#include "mbed.h"

void init_external_payload();
void payload_handler();



void SEND_TO_EXTERNAL_PAYLOAD(char *data, uint8_t len);
void EXTERNAL_PAYLOAD_LATEST_BYTES(char *_params);
void SET_EXTERNAL_PAYLOAD_PASSTHROUGH(char *_params);
void ENABLE_EXTERNAL_PAYLOAD_RECORDING(char *_params);
void DISABLE_EXTERNAL_PAYLOAD_RECORDING(char *_params);
void EXTERNAL_PAYLOAD_RECORDING_SIZE(char *_params);

#endif // __EXTERNAL_PAYLOAD_H