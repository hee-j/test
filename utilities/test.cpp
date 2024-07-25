#include "test.h"
#include "submodule.h"
#include "legacy_comm.h"
/* 
    Subsystemstatus bit=0 - nominal result, bit=1 - test failed

    bits LSB->MSB:
    0 - SD-card
    1 - Camera
    2 - GPS
    3 - BMP
    4 - IMU
    5 - Radio
    6 - ADC
    7 - BT

*/

char subSystemStatus = 0b00000000; // LSB->MSB: SD-card, cam, gps, bmp, imu, radio, adc, bt

char set_subsystem_status(char subsystem, char status, bool printresult)
{
    if(status!=0)
    {
     subSystemStatus |= 1 << subsystem;   
    }else
    {
     subSystemStatus &= ~(1 << subsystem);  
    }
    if(printresult)
    {
        printf(subSystemStatus ? "Failed\r\n" :  "Success\r\n" );
    }
    return subSystemStatus;
}


int get_subsystem_status(char subsystem)
{
    return (subSystemStatus >> subsystem) & 1;
}

int system_has_errors()
{
    return subSystemStatus != 0;
}


void GET_SYSTEM_STATUS(){
    char answer[4];
    sprintf(answer, "%c", subSystemStatus);
    sendPacket(OBC, 13, answer, 4);
}


