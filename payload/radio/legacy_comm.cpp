#include "legacy_comm.h"
// TODO: this source file is partially deprecated. Please update.

#include <string>
#include "loop.h"
#include "m_util.h"
#include "settings.h"
#include "serial.h"
#include "timers.h"
#include "../utilities/serial/logging.h"
// Settings.cpp
extern char transmissionMode;

int transmissionEnabled(char _transmissionMode, char transmissionModeBit);
void buildPacket(char *buf, char targetID, char commandID, char *data, int data_len, int timestamp);

char setTransmissionMode(char _transmissionMode, char bitToSet, char bitValue)
{
    if(bitValue=='1')
    {
     _transmissionMode |= 1 << (bitToSet-'0');   
    }else if(bitValue=='0')
    {
     _transmissionMode &= 1 << (bitToSet-'0');  
    }
    return _transmissionMode;
}

int transmissionEnabled(char _transmissionMode, char transmissionModeBit)
{
    return (_transmissionMode >> transmissionModeBit) & 1;
}

void _message(const char *message, uint8_t subsystemID, uint8_t commandID)
{
    int message_length = strlen(message);
    if(message_length>244)
    {
      message_length = 244;   
    }
    sendPacket(subsystemID, commandID, (char*) message, message_length);
}

void raw_downlink(char* buf, int length) {
    //radio_tx(buf, length);
    _error("RADIO TX NOT YET IMPLEMENTED.\n\r");
}


void sendPacketRealTime(char subsystemID, char commandID, char *data, int data_len)
{
    sendPacketRealTime(subsystemID, commandID, data, data_len, ms_since_boot());
}

void sendPacketRealTime(char subsystemID, char commandID, char *data, int data_len, int timestamp)
{
    char packet[data_len+11];
    memset(packet, 0, sizeof(packet));
    buildPacket(packet, subsystemID, commandID, data, data_len, timestamp);
    directTransmission(packet, data_len+11);
}

void sendPacket(char subsystemID, char commandID, char *data, int data_len)
{
    sendPacket(subsystemID, commandID, data, data_len, ms_since_boot());
}

void sendPacket(char subsystemID, char commandID, char *data, int data_len, int timestamp) //TODO add the other transmission modes
{
    char packet[data_len+11];
    memset(packet, 0, sizeof(packet));
    buildPacket(packet, subsystemID, commandID, data, data_len, timestamp);

    if(transmissionEnabled(transmissionMode, COMMUNICATION_MODE_UART))
    {
        pc_printf("packet:");
        for(int i=0; i<data_len+11; i++)
        {
          pc_putc(packet[i]);   
        }
        pc_printf("\n\r");
    }

    if(transmissionEnabled(transmissionMode, COMMUNICATION_MODE_RADIO))
    {
        // NOTE: Radio disabled
        transmitPacketRF(packet, data_len+11);
        //radio_tx(packet, data_len+11);
        //_error("RADIO TX NOT YET IMPLEMENTED.\n\r");
    }

}

uint32_t hecking_fnv(char *bytes, int str_len)
{
  uint32_t __hval = 0x811c9dc5;
  uint32_t __fnv_32_prime = 0x01000193;
  uint64_t __uint32_max = 4294967296;
  for(int i = 0; i< str_len; i++)
  {
    __hval = __hval ^ bytes[i];
    __hval = (__hval * __fnv_32_prime) % __uint32_max; 
  }
  
  return __hval;
}


void buildPacket(char *buf, char targetID, char commandID, char *data, int data_len, int timestamp)
{
    buf[0] = targetID;
    //_pc.printf("Target id: %d\n\r", buf[0]);
    buf[1] = commandID;
    //_pc.printf("Command id: %d\n\r", buf[1]);
    memcpy(buf+2, &data_len, 1);
    //_pc.printf("Data len: %d\n\r", buf[2]);
    memcpy(buf+3, &timestamp, 4);
    //_pc.printf("Timestamp: %d\n\r", timestamp);
    memcpy(buf+7, data, data_len);
    //_pc.printf("Data: %s\n\r", data);
    uint32_t m_fnv = hecking_fnv(buf, data_len+7);
    //_pc.printf("THe hecking FNV: %lu\n\r", m_fnv);
    memcpy(buf+(data_len+7), (uint32_t*) &(m_fnv), 4);
}