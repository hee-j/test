#include "radio.h"
#include "m_util.h"
#include "settings.h" // Radio configuration
#include <string>
#include <queue>
#include "RadioController.h"
#include "logging.h"
#include "RFM69/RFM69.h"
#include "submodule.h"
#include "legacy_comm.h"
#include "errors.h"

DigitalOut      radio_rst   (RADIO_RST);
extern RFM69    radio;
extern char     radio_data[64];

void radio_reset(void)
{
    radio_rst = 1;
    ThisThread::sleep_for(100ms);
    radio_rst = 0;
    ThisThread::sleep_for(200ms);
}

uint8_t radio_get_temp()
{
  _debug("Radio temperature: %f\n\r", radio.readTemperature(-1));
  return radio.readTemperature(-1);
}

int radio_get_RSSI()
{
  if(!radio.RSSI)
  {
    _debug("No RSSI\n\r");
    return 0xFFFFF;
  }
  _debug("RSSI: %d\n\r", radio.RSSI);
  return radio.RSSI;
}

char radio_init(void)
{
    _debug("Loading radio module... ");
    radio_rst = 0;
    //radio_reset();
    //wait(1.0);
    
    if (! initialize_radio(2) )
    {
        _error("Fail :(\n\r");
        radio_reset();
        _debug("Restarting radio...\n\r");
        ThisThread::sleep_for(500ms);
        
        if( ! initialize_radio(2))
        {
            _error("Still unable to reset radio. Check your wiring and battery voltage.\n\r");
            return RADIO_LOAD_FAILED;
        }
    }
    else {
    _ack("OK\n\r");
    }
      
    return 0;
}

void createRandomMessage(char *message, int length, int index)
{
    message[0] = 'm';
    
    for(int i = 9; i<length; i++){
        message[i] = rand()%128;
    }

    uint32_t _fnv = ufnv(message+9, length-9);

    memcpy(message+1, &_fnv, 4);
    memcpy(message+5, &index, 4);
}

// Command interface

void TEMP_RF()
{
    char floatbuf[16];
    memset(floatbuf,0,sizeof(floatbuf));

    sprintf(floatbuf, "%5.2f", (float)radio_get_temp());
    sendPacket(RF, 1, floatbuf, strlen(floatbuf));
}
void RSSI_RF()
{
    char rssibuf[4];
    memset(rssibuf,0,sizeof(rssibuf));

    sprintf(rssibuf, "%d", radio_get_RSSI());
    sendPacket(RF, 2, rssibuf, strlen(rssibuf));
}

void RF_ALL()
{
    radio_get_RSSI();
    radio_get_temp();

    // Send packets...
}