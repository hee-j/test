#ifndef __RADIO_CONTROLLER_H
#define __RADIO_CONTROLLER_H

#include "mbed.h"
extern char         radio_data[64];
extern uint8_t      radio_data_length;
extern uint8_t      RADIO_NEW_DATA;
extern int16_t      RSSI;
extern uint8_t      transmitqueue_length;

void createRandomMessage(char *message, int length, int index);
int initialize_radio(int baud);
void transmitPacketRF(char *_data, int _length);
void directTransmission(char *data, int length);

void    transmit        (void);

void radioControl();

void read_radio_data();
void setNetworkID(uint8_t id);

struct dataPacket{
    char data[64];
    uint8_t length;
};

#endif