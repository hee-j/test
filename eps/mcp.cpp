#include "mcp.h"
#include "mcp3008/mcp3008.h"
#include "hw_config.h"

#include "flags.h"
#include "submodule.h"

#include "legacy_comm.h"
#include "settings.h"
#include "logging.h"

SPI spi2(MCP_SPI_MOSI, MCP_SPI_MISO, MCP_SPI_SCK);
MCP3008 mcp(&spi2, MCP_CS);

DigitalIn eps_charging_status(CHARGING_STATUS);
DigitalOut mcp_cs(MCP_CS);

#define CHANNEL_BATT_CURRENT    0
#define CHANNEL_PANEL_CURRENT   1
#define CHANNEL_VOLTAGE_XM      2
#define CHANNEL_VOLTAGE_YM      4
#define CHANNEL_VOLTAGE_XP      3
#define CHANNEL_VOLTAGE_YP      5
#define CHANNEL_VOLTAGE_BATT    6
#define CHANNEL_NOT_CONNECTED   7

#if defined(EPS_VERSION_1_4)
float channel_gain[] = {0.57232049794, 0.57232049794, 0.0064516129, 0.0064516129, 0.0064516129, 0.0064516129, 0.0064516129, 1};
float channel_bias[] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif
#if defined(EPS_VERSION_BELOW_1_4)
float channel_gain[] = {0.57232049794, 0.57232049794, 0.0064516129, 0.0064516129, 0.0064516129, 0.0064516129, 0.0064516129, 1};
float channel_bias[] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif

float read_channel(uint8_t channel)
{
    if(channel > 7) return 0;
    int value = mcp.read(channel);
    return value*channel_gain[channel] + channel_bias[channel];
}

int adc_init(void)
{
    // disable mcp
    mcp_cs = 1;

    return 0;

    _debug("ADC subsystem loaded.\n\r");
}

double measure_and_average(int channel, int times)
{
    if(times < 3) times = 3;
    float meas[times];
    int lowestIndex = 0;
    float lowest = (float)0x7f7fffff;
    int highestIndex  = 0;
    float highest = 0;
    float value = 0;
    for(int i = 0; i<times; i++)
    {
        value = read_channel(channel);
        if(value > highest)
        {
            highest = value;
            highestIndex = i;
        }
        if(value < lowest)
        {
            lowest = value;
            lowestIndex = i;
        }
        meas[i] = value;
    }

    float sum = 0;
    for(int i = 0; i<times; i++)
    {
        if(i != lowestIndex || i != highestIndex)
        {
            sum += meas[i];
        }
    }

    return (double) sum / (double)(times-2);
}

/*
void adc_measure()
{
    float meas[8];
    char kaikki[64];
    memset(kaikki, 0, sizeof(kaikki));
    
    char buf[8];
    char* meas_ptr = kaikki;
    for(int i = 0; i<8; i++)
    {
      meas[i] = (uint16_t) measure_and_average(i, 1);
      i < 7 ? sprintf(buf, "%5.2f,", meas[i]) : sprintf(buf, "%5.2f", meas[i]);
      strncpy(meas_ptr, buf, sizeof(buf));
      meas_ptr += strlen(meas_ptr);
    }

    //_debug("MCP: %s\n\r", kaikki);

    sendPacket(EPS, 4, kaikki, strlen(kaikki));
}
*/

// Command interface

void BATT_V(char* _params)
{
    char resp[16];
    memset(resp,0,sizeof(resp));
    
    // TODO: params
    float v = read_channel(CHANNEL_VOLTAGE_BATT);

    sprintf(resp, "%5.2f", v);
    
    sendPacket(EPS, 1, resp, strlen(resp));
}

void SP_V(char* params)
{
    char resp[32];
    memset(resp,0,sizeof(resp));
    
    float x_minus = read_channel(CHANNEL_VOLTAGE_XM);
    float x_plus = read_channel(CHANNEL_VOLTAGE_XP);
    float y_minus = read_channel(CHANNEL_VOLTAGE_YM);
    float y_plus = read_channel(CHANNEL_VOLTAGE_YP);
    sprintf(resp, "%5.2f,%5.2f,%5.2f,%5.2f", x_minus, x_plus, y_minus, y_plus);
    
    sendPacket(EPS, 2, resp, strlen(resp));
}

void SP_C(char* params)
{
    char resp[16];
    memset(resp,0,sizeof(resp));
    float sp_current = read_channel(CHANNEL_PANEL_CURRENT);
    float vx = read_channel(CHANNEL_VOLTAGE_XM) + read_channel(CHANNEL_VOLTAGE_XP);
    float vy = read_channel(CHANNEL_VOLTAGE_YM) + read_channel(CHANNEL_VOLTAGE_YP);
    float cx = sp_current * (vx/(vx + vy));
    float cy = sp_current * (vy/(vx + vy));
    sprintf(resp, "%5.2f,%5.2f", cx, cy);
    
    sendPacket(EPS, 3, resp, strlen(resp));
}

void EPS_MEASURE_ALL()
{
    float meas[8];
    char kaikki[64];
    memset(kaikki, 0, sizeof(kaikki));
    
    char buf[8];
    char* meas_ptr = kaikki;
    for(int i = 0; i<8; i++)
    {
        if(i == 0){ meas[i] = measure_and_average(0, 20); }
        else{ meas[i] = read_channel(i); }
      
      i < 7 ? printf("%5.2f,", meas[i]) : printf("%5.2f\r\n", meas[i]);
      i < 7 ? sprintf(buf, "%5.2f,", meas[i]) : sprintf(buf, "%5.2f", meas[i]);
      strncpy(meas_ptr, buf, sizeof(buf));
      meas_ptr += strlen(meas_ptr);
    }

    //_pc.printf("EPS values: (battery voltage, solar panel current, solar panel voltage): %s\n\r", kaikki);
    //_debug("EPS values: (battery voltage, solar panel current, solar panel voltage): %s\n\r", kaikki);
    sendPacket(EPS, 4, kaikki, strlen(kaikki));
}

void GET_CHARGING_STATUS()
{
    uint8_t status = 0;
    status = eps_charging_status; // read the GPIO in
    status = !status; // reverse to coincide with the protocol
    char answer[4];
    sprintf(answer, "%d", status);

    sendPacket(EPS,5,answer,strlen(answer));
}