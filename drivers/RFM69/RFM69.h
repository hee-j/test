// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright Felix Rusu (2014), felix@lowpowerlab.com
// http://lowpowerlab.com/
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General    
// Public License as published by the Free Software       
// Foundation; either version 3 of the License, or        
// (at your option) any later version.                    
//                                                        
// This program is distributed in the hope that it will   
// be useful, but WITHOUT ANY WARRANTY; without even the  
// implied warranty of MERCHANTABILITY or FITNESS FOR A   
// PARTICULAR PURPOSE. See the GNU General Public        
// License for more details.                              
//                                                        
// You should have received a copy of the GNU General    
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//                                                        
// Licence can be viewed at                               
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#ifndef RFM69_h
#define RFM69_h
#include <mbed.h>            // assumes Arduino IDE v1.0 or greater
#include "../../utilities/hw_config.h"

#define RF69_MAX_DATA_LEN       64 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
//#define RF69_SPI_CS             SS // SS is the SPI slave select pin, for instance D10 on ATmega328

#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR 255
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000
#define RF69_FSTEP  61.03515625 // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet)

void isr0(void);
extern volatile uint8_t RADIO_IN_USE;

class RFM69 {
  public:
    static volatile uint8_t DATA[RF69_MAX_DATA_LEN]; // recv/xmit buf, including header & crc bytes
    static volatile uint8_t DATALEN;
    static volatile uint8_t SENDERID;
    static volatile uint8_t TARGETID; // should match _address
    static volatile uint8_t PAYLOADLEN;
    static volatile uint8_t ACK_REQUESTED;
    static volatile uint8_t ACK_RECEIVED; // should be polled immediately after sending a packet with ACK request
    static volatile int16_t RSSI; // most accurate RSSI during reception (closest to the reception)
    static volatile uint8_t _mode; // should be protected?  

    RFM69(PinName mosi, PinName miso, PinName sclk, PinName slaveSelectPin, PinName interrupt);

    bool initialize(uint8_t freqBand, uint8_t ID, uint8_t RF_BITRATE=3, uint8_t networkID=1);
    void setAddress(uint8_t addr);
    void setNetwork(uint8_t networkID);
    bool canSend();
    void send(uint8_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK=false);
    bool sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries=2, uint8_t retryWaitTime=40); // 40ms roundtrip req for 61byte packets
    bool receiveDone();
    bool ACKReceived(uint8_t fromNodeID);
    bool ACKRequested();
    void sendACK(const void* buffer = "", uint8_t bufferSize=0);
    uint32_t getFrequency();
    void setFrequency(uint32_t freqHz);
    void encrypt(const char* key);
//    void setCS(uint8_t newSPISlaveSelect);
    int16_t readRSSI(bool forceTrigger=false);
    void promiscuous(bool onOff=true);
    void setBaudRate(uint8_t RF_BITRATE);
    void setHighPower(bool onOFF=true); // has to be called after initialize() for RFM69HW
    void setPowerLevel(uint8_t level); // reduce/increase transmit power level
    void sleep();
    uint8_t readTemperature(int8_t calFactor=0); // get CMOS temperature (8bit)
    void rcCalibration(); // calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]
    int hasNewData(void);
    // allow hacking registers by making these public
    uint8_t readReg(uint8_t addr);
    void writeReg(uint8_t addr, uint8_t val);
    void readAllRegs();
    void receiveBegin();

  protected:
    void virtual interruptHandler();
    void sendFrame(uint8_t toAddress, const void* buffer, uint8_t size, bool requestACK=false, bool sendACK=false);

    static RFM69* selfPointer;
    DigitalOut _slaveSelectPin;
    uint8_t _address;
    Timer t;
    bool _promiscuousMode;
    uint8_t _powerLevel;
    bool _isRFM69HW;
    SPI _spi;
    void setMode(uint8_t mode);
    void setHighPowerRegs(bool onOff);
    void select();
    void unselect();
};

#endif
