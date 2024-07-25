#ifndef __CLI_H_
#define __CLI_H_


// WARNING: NOT YET IMPLEMENTED.


#include "mbed.h"
#include "cmd_interpreter.h"
#include "settings.h"
#include "cli_command_parser.h"
#include "serial.h"

void    setup_cli       ();
void    serial_callback ();
int     command_callback();
void    mread_serial(void);

#define READ_TIMEOUT            3.0 // 2 seconds

//#define __C_SHARP_DESKTOP_UI_   1
/****
#define HELP_STR "------------ Command list ------------\n\r \
All commands are case-sensitive.\n\r\n\r \
\
\n\rBASICS\n\r \
ping\t\t\tPing the satellite\n\r \
\n\rIMAGERY COMMANDS\n\r \
take_pic\t\tTake picture\n\r \
list\t\t\t<n> latest pictures. Defaults to 1.\n\r \
num_blocks\t\tReturns the amount of blocks in image.\n\r \
get_blocks\t\tReturns a 64-bit sized chunk of image.\n\r \
get_latest\t\tReturns the name of latest image.\n\r \
\n\rBROADCASTING\n\r \
beep\t\t\tBeep <times>. Defaults to 1.\n\r \
morse\t\t\tMorse <string>.\n\r \
radio_tx\t\tSend <string> to the radio network.\n\r \
\n\rPAYLOAD DATA\n\r \
gps_get\t\tRead GPS time, location, velocity, altitude, and fix.\n\r \
bmp_get\t\tRead BMP values (temp, press). Defaults to 'all'.\n\r \
rf_get\t\t\tRead radio housekeeping values (temp, RSSI). Defaults to 'all'.\n\r \
imu_get\t\tRead accelerometer, gyroscope, and magnetometer sensors.\n\r \
eps_get\t\tRead battery voltage, solar panel voltage, and solar panel current. Defaults to 'all'.\n\r \
\n\rCONFIGURATION\n\r \
imu_calib\t\tCalibrate accelerometer, gyroscope, and magnetometer sensors.\n\r \
whoami\t\t\tFind out your identity.\n\r \
get_node_id\t\tFind out your node ID on the RF69 network.\n\r \
\n\rLEDs\n\r \
led_on\t\t\tTurns led <i> on. Defaults to 'all'. Optionally, sets led <i> to <r> <g> <b> color (RGB values ranging from 0 to 10,000).\n\r \
led_off\t\tTurns led <i> off. Defaults to 'all'.\n\r \
\n\rSYSTEM\n\r \
reset\t\t\tResets the device\n\r \
mount\t\t\tMount file system.\n\r \
umount\t\t\tUnmount file system. (Done automatically during restart.)\n\r \
is_mounted\t\tCheck if fs is mounted.\n\r \
help\t\t\tLists available commands\n\r ----------------------------------------\n\r"

void    print_help_str(void);***/

#endif

// Add these once tested:
/*****
led <x> on\t\tLit LED 1, 2 or 3. Defaults to 'all'.\n\r \
led <x> off\t\tTurn off LED 1, 2 or 3. Defaults to 'all'.\n\r \
load_pic\t\tLoad picture\n\r \
*****/