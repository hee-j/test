#pragma once
#ifndef __LEGACY_SERIAL_H_
#define __LEGACY_SERIAL_H_

// TODO: this is deprecated. Please update.

#include "mbed.h"

char setTransmissionMode(char _transmissionMode, char bitToSet, char bitValue);

//void _debug(char *message); // moved to logging.h
void sendPacketRealTime(char subsystemID, char commandID, char *data, int data_len);
void sendPacketRealTime(char subsystemID, char commandID, char *data, int data_len, int timestamp);
void sendPacket(char subsystemID, char commandID, char *data, int data_len);
void sendPacket(char subsystemID, char commandID, char *data, int data_len, int timestamp);
void _message(const char *message, uint8_t subsystemID=12, uint8_t commandID=1);

int transmissionEnabled(char _transmissionMode, char transmissionModeBit);

#define COMMUNICATION_MODE_UART     0
#define COMMUNICATION_MODE_RADIO    1
#define COMMUNICATION_MODE_LOG      2
#define COMMUNICATION_MODE_BT       3
#define COMMUNICATION_MODE_DEBUG    4

#define    TEST_FLAG       1
#define    REBOOT_FLAG     2
#define    RECOVERY_FLAG   4

#endif