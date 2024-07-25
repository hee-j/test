#ifndef __SUBMODULE_H_
#define __SUBMODULE_H_

// For the latest command definitions, refer to Kitsat Com Protocol in Google sheets:
// https://docs.google.com/spreadsheets/d/10JQrkpLVDIyB03xetAFTzuSk0sgWTfwx5vMFuJEL8vY/edit?ouid=103140426699801337154&usp=sheets_home&ths=true

// Latest command indices
#define IMU_MAG     1
#define IMU_GYR     2
#define IMU_ACC     3
#define IMU_CALIB   13
#define IMU_ALL     14

#define ADC_ALL     1

// Subsystem IDs in the command interface
#define OBC 1
#define CAM 2
#define GPS 3
#define _BMP 4
#define IMU 5
#define RF 6
#define BUZZER 7
#define EPS 8
#define LED 9
#define GENERAL 10
#define GS 11
#define DEBUG 12
#define BT 13
#define SETTINGS 15
#define FILETRANSFER 16
#define EXTERNAL_PAYLOAD 17

// IMU commands
#define mag_all 1
#define mag_x 2
#define mag_y 3
#define mag_z 4
#define gyr_all 5
#define gyr_x 6
#define gyr_y 7
#define gyr_z 8
#define acc_all 9
#define acc_x 10
#define acc_y 11
#define acc_z 12
#define calib 13
#define imu_all 14

// EPS Commands
#define battery_voltage 1
#define X_minus_voltage 2
#define X_voltage 3
#define Y_minus_voltage 4
#define Y_voltage 5
#define X_current 6
#define Y_current 7

// LED commands
#define LED_stop 1
#define LED_start 2
#define LED_mode 3

// CAM commands
#define cam_list_n  1
#define cam_num_blocks 2
#define cam_get_blocks 3
#define cam_take_pic 4
#define cam_reset 5
#define cam_ping 6
#define cam_latest 7

#endif