#include "gps.h"
#include "serial.h"
#include "../utilities/submodule.h"
#include "../cmd_interpreter.h"
#include "radio/legacy_comm.h"

UnbufferedSerial            gpsSerial(GPS_TX, GPS_RX);
GePeS             myGPS;

#define         SLEEP_TIME                  2000 // (msec)
#define         PRINT_AFTER_N_LOOPS         20

extern uint8_t  __GPS_FLAG;

void GPS_init(void)
{
    // <-- Initialize
    GPS_begin(&gpsSerial, &myGPS);
    // -->
    _debug("GPS subsystem loaded.\n\r");
}

void GPS_read_all(void)
{
    char floatBuf[16];
    memset(floatBuf, 0, sizeof(floatBuf));

    if(myGPS.fix)
    {
        float values[] = {myGPS.latitude, myGPS.longitude, myGPS.altitude, myGPS.speed };
            
        for(int i=0; i<4; i++)
        {
            memcpy(floatBuf+(i*4), values+i, 4);
        }
        printf("%s", floatBuf);
        sendPacket(GPS, 6, floatBuf, 16);
    }else{
        printf("nofix");
        _message("nofix", GPS, 6);
    }
}

void gps_read_velocity(char* buf)
{
    sprintf((char*)buf,"%5.2f", myGPS.speed);
}

void gps_read_altitude(char* buf)
{
    sprintf((char*)buf,"%5.2f", myGPS.altitude);
}

void gps_read_fix(char* buf)
{
    sprintf((char*)buf,"%d", myGPS.fix);
}

void gps_read_location(char* buf)
{
    sprintf((char*)buf,"%5.2f,%5.2f,%5.2f", myGPS.latitude, myGPS.longitude, myGPS.altitude);
}

void gps_read_time(char* buf)
{
    sprintf((char*)buf, "%02d:%02d:%02d", myGPS.hour, myGPS.minute, myGPS.seconds);
}

void gps_read_date(char *buf)
{
    sprintf((char*)buf, "%d-%d-%d", myGPS.year, myGPS.month, myGPS.day);
}

// COMMAND INTERFACE

void GPS_TIME(char* _params)
{
    char answer[32];
    gps_read_time(answer);
    sendPacket(GPS, 1, answer, strlen(answer));
}

void GPS_LOCATION(char* _params)
{
    char answer[32];
    gps_read_location(answer);
    sendPacket(GPS, 2, answer, strlen(answer));
}

void GPS_VELOCITY(char* _params)
{
    char answer[32];
    gps_read_velocity(answer);
    sendPacket(GPS, 3, answer, strlen(answer));
}

void GPS_ALTITUDE(char* _params)
{
    char answer[32];
    gps_read_altitude(answer);
    sendPacket(GPS, 4, answer, strlen(answer));
}

void GPS_FIX(char* _params)
{
    char answer[32];
    gps_read_fix(answer);
    sendPacket(GPS, 5, answer, strlen(answer));
}

void GPS_ALL(char *_params)
{
    GPS_read_all();
}

void GPS_DATE(char *_params)
{
    char answer[32];
    gps_read_date(answer);
    sendPacket(GPS, 7, answer, strlen(answer));
}