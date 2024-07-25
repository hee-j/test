#include "RadioController.h"
#include "RFM69/RFM69.h"
#include <chrono>
#include <queue>
#include "settings.h"
#include "radio_indicators.h"
#include "main.h"
#include "../../utilities/serial/logging.h"

RFM69 radio(RADIO_MOSI, RADIO_MISO, RADIO_SCLK, RADIO_CS, RADIO_DIO0);

queue<dataPacket> transmitQueue; 


char            radio_data[64];
uint8_t         radio_data_length = 0;
uint8_t         RADIO_NEW_DATA = 0;
uint8_t         network_id = 101;
uint8_t         transmitqueue_length = 0;
volatile uint8_t TRANSMIT_QUEUE_ACTIVE = 0;
Timer           transmitTimer;
int16_t         RSSI;


uint8_t TRANSMIT_FLAG       = 0;
void transmit(void)
{
    _debug("Transmitting on \r\n");
    TRANSMIT_FLAG = 1;
}

void setNetworkID(uint8_t id){
    radio.setNetwork(id);
}

int initialize_radio(int baud)
{
    // initialize radio
    if(!radio.initialize(FREQUENCY, NODE_ID, baud, network_id))
    {
        return 0;
    }
    
    radio.encrypt(0); //disable encryption to allow larger packages
    radio.promiscuous(0);
    radio.setHighPower(true); // turn the PA stages on - necessary even for ISM usage
    radio.setPowerLevel(MAX_POWER_LEVEL);
    radio.receiveBegin();
    memset(radio_data, 0, sizeof(radio_data));
    transmitTimer.start();


    return 1;
}

void transmitPacketRF(char *_data, int _length)
{
    dataPacket _packet;
    _packet.length = _length;
    memcpy(_packet.data, _data, _length > 64 ? 64 : _length);
    transmitQueue.push(_packet);
    transmitqueue_length = transmitQueue.size();
    TRANSMIT_QUEUE_ACTIVE = 1;
}

void read_radio_data()
{
   //_pc.printf("That's your data\n\r");
}


void directTransmission(char *data, int length)
{
    radio.send(0, data, length);
}


void radioControl(){
    if(TRANSMIT_QUEUE_ACTIVE){
        
        while(chrono::duration_cast<chrono::milliseconds>(transmitTimer.elapsed_time()).count() < 5);

        if(transmitQueue.size() > 0){
            dataPacket _packet = transmitQueue.front();
            transmitQueue.pop();
            transmitqueue_length--;
            radio.send(0, _packet.data, _packet.length);
            transmitTimer.reset();
        }
        if(!(transmitQueue.size() > 0)){
            TRANSMIT_QUEUE_ACTIVE = 0;
            radio.receiveBegin();
        }
    }
    if(radio.hasNewData())
    {
        transmitTimer.reset();
        int j = radio.DATALEN;
        char data[64];
        memset(data,0,sizeof(data));
        for(int i = 0; i<j; i++)
        {
            data[i] = radio.DATA[i];
        }
        int fnv_result = checkFNV(data, j);
        if(fnv_result)
        {
            radio_data_length = j;
            memcpy(radio_data, data, 64);
            RADIO_NEW_DATA = 1;
            radio_received();
        }
        RSSI = radio.RSSI;
        radio.receiveBegin();
    }

    
}
