#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include "mbed.h"
#include "m_util.h"
#include <stdio.h>
#include <stdlib.h>
#include "../payload/radio/RadioController.h"

// Radio configuration
#define GATEWAY_ID      2
//#define NETWORKID       101   //the same on all nodes that talk to each other
#define NODE_ID         101    // node ID
#define FREQUENCY       RF69_433MHZ
#define RFBUFSIZE       64   // message buffersize
#define FREQUENCY       RF69_433MHZ
#define ISM_MAX_POWER_LEVEL     25
#define MAX_POWER_LEVEL         31

// Hard-coded flight mode because mbed studio doesn't let you set compiler options
//#define HARD_FLIGHT_MODE    1

//#define PL1_4
#define PL1_3
//#define PL1_2
//#define PL_custom
#ifdef PL1_4
    #define IMU_IS_ICM_AND_MMC          1
    #define ENV_IS_BMP384               1
    #define LED_DRIVER_IS_STP           1
#endif

#ifdef PL1_3
    #define IMU_IS_BMX055               1
    #define ENV_IS_DPS310               1 
    #define LED_DRIVER_IS_TLC           1
#endif

#ifdef PL1_2
    #define IMU_IS_LSM9DS1              1
    #define ENV_IS_BMP280               1
    #define LED_DRIVER_IS_TLC           1
#endif

#ifdef PL_custom
    #define IMU_IS_LSM9DS1              1
    #define ENV_IS_DPS310               1
    #define LED_DRIVER_IS_STP           1
#endif


//#define EPS_VERSION_BELOW_1_4
#define EPS_VERSION_1_4 1

void    SET_FLIGHT_MODE         (char *_params);
int     get_image_index         (void);
int     create_user_directory   (void);
int     read_node_id_from_file  (void);
int     write_node_id_to_file   (int node_id);
int     read_flight_mode_from_file(void);
int     write_flight_mode_to_file(int status);
int     read_intensive_flight_mode_from_file(void);
int     write_intensive_flight_mode_to_file(int status);

#endif