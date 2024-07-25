#ifndef __K_IMU_H_
#define __K_IMU_H_

#include "mbed.h"
#include <stdlib.h>

#include "lsm9ds1/LSM9DS1.h"
#include "BMX055.h"
#include "ICM-20608-G/ICM-20608-G.h"
#include "MMC5633NJL/mmc5633njl.h"

#include "submodule.h"
#include "flags.h"
#include "logging.h"

int     imu_init        (void);


uint8_t imu_read_acc(float *values);
uint8_t imu_read_mag(float *values);
uint8_t imu_read_gyro(float *values);
void imu_calibrate();


// Command interface
void _IMU_MAG       ();
void _IMU_GYR       ();
void _IMU_ACC       ();
void _IMU_CALIB     ();
void _IMU_ALL       ();


#endif