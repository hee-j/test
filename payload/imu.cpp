#include "imu.h"
#include "legacy_comm.h"
#include <cstdio>
#include <cstring>
#include "settings.h"
#include "hw_config.h"

#ifdef IMU_IS_BMX055
BMX055          imu_bmx(IMU_I2C_SDA, IMU_I2C_SCL);

BMX055_ACCEL_TypeDef  acc;
BMX055_GYRO_TypeDef   gyr;
BMX055_MAGNET_TypeDef mag;

const BMX055_TypeDef bmx055_my_parameters = {
   // ACC
   ACC_2G,
   ACC_BW250Hz,
   // GYR
   GYR_125DPS,
   GYR_200Hz23Hz,
   // MAG
   MAG_ODR10Hz
 };
#endif
#ifdef IMU_IS_ICM_AND_MMC
#include "ICM-20608-G/ICM-20608-G.h"
#include "MMC5633NJL/mmc5633njl.h"
ICM_20608_G imu_icm(G_1000_DEGS, A_8_G, IMU_I2C_SDA, IMU_I2C_SCL);
#endif
#ifdef IMU_IS_LSM9DS1
LSM9DS1         imu_lsm(D14, D15, 0xD6, 0x3C);
#endif
extern uint8_t  __IMU_FLAG;

int imu_init(void)
{
    #ifdef IMU_IS_LSM9DS1
        return imu_lsm.begin();
        
    #endif
    #ifdef IMU_IS_BMX055
        return imu_bmx.set_parameter(&bmx055_my_parameters);
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
        uint8_t result = setup_mmc5633();
        mmc5633_whoami();
        result |= imu_icm.init();
        return result;
    #endif
}



uint8_t imu_read_acc(float *values)
{
    #ifdef IMU_IS_LSM9DS1
        imu_lsm.readAccel();
        values[0] = imu_lsm.calcAccel(imu_lsm.ax);
        values[1] = imu_lsm.calcAccel(imu_lsm.ay);
        values[2] = imu_lsm.calcAccel(imu_lsm.az);
        return 0;
    #endif
    #ifdef IMU_IS_BMX055
        BMX055_ACCEL_TypeDef acc;
        imu_bmx.get_accel(&acc);
        values[0] = acc.x;
        values[1] = acc.y;
        values[2] = acc.z;
        return 0;
    #endif
    
    #ifdef IMU_IS_ICM_AND_MMC
        imu_icm.readAccel(values[0], values[1], values[2]);
        return 0;
    #endif
    return 1;
}

uint8_t imu_read_mag(float *values)
{
    #ifdef IMU_IS_LSM9DS1
        imu_lsm.readMag();
        values[0] = imu_lsm.calcMag(imu_lsm.mx);
        values[1] = imu_lsm.calcMag(imu_lsm.my);
        values[2] = imu_lsm.calcMag(imu_lsm.mz);
        return 0;
    #endif
    #ifdef IMU_IS_BMX055
        BMX055_MAGNET_TypeDef mag;
        imu_bmx.get_magnet(&mag);
        values[0] = mag.x;
        values[1] = mag.y;
        values[2] = mag.z;
        return 0;
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
        float temp;
        read_mmc5633(values, values+1, values+2, &temp);
        return 0;
    #endif
    return 1;
}

uint8_t imu_read_gyro(float *values)
{
    #ifdef IMU_IS_LSM9DS1
        imu_lsm.readGyro();
        values[0] = imu_lsm.calcGyro(imu_lsm.gx);
        values[1] = imu_lsm.calcGyro(imu_lsm.gy);
        values[2] = imu_lsm.calcGyro(imu_lsm.gz);
        return 0;
    #endif
    #ifdef IMU_IS_BMX055
        BMX055_GYRO_TypeDef gyr;
        imu_bmx.get_gyro(&gyr);
        values[0] = gyr.x;
        values[1] = gyr.y;
        values[2] = gyr.z;
        return 0;
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
        imu_icm.readGyro(values[0], values[1], values[2]);
        return 0;
    #endif
    return 1;
}


void imu_calibrate()
{
    #ifdef IMU_IS_LSM9DS1
        imu_lsm.calibrate();
    #endif
    #ifdef IMU_IS_BMX055
        printf("CALIBRATION NOT IMPLEMENTED");
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
        imu_icm.calibrate();
    #endif
    _debug("IMU Calibration succesful\n\r");
}

// Command interface

void _IMU_MAG()
{
    float values[3];
    imu_read_mag(values);
    char msg[32];
    sprintf(msg, "%4.2f, %4.2f, %4.2f", values[0], values[1], values[2]);
    sendPacket(IMU, 1, msg, strlen(msg));
}

void _IMU_GYR()
{
    float values[3];
    imu_read_gyro(values);
    char msg[32];
    sprintf(msg, "%4.2f, %4.2f, %4.2f", values[0], values[1], values[2]);
    sendPacket(IMU, 5, msg, strlen(msg));
}

void _IMU_ACC()
{
    float values[3];
    imu_read_acc(values);
    char msg[32];
    sprintf(msg, "%4.2f, %4.2f, %4.2f", values[0], values[1], values[2]);
    sendPacket(IMU, 9, msg, strlen(msg));
}

void _IMU_CALIB()
{
    imu_calibrate();
}

void _IMU_ALL()
{
    float values[9];
    imu_read_mag(values);
    imu_read_gyro(values+3);
    imu_read_acc(values+6);

    sendPacket(IMU, 14, (char *)values, 36);

}


/*
int imu_ready(void)
{
    int err = 0;
    #ifdef IMU_IS_BMX055
        if (imu.chip_ready() == 0)
        {
            err = -1;
            printf("Bosch BMX055 is NOT available!\r\n");
        }
    #endif
    return err;
}
*/
/*
// TODO: deprecated function
void imu_read_automatic(float values[])
{
    #ifdef IMU_IS_BMX055
    imu.get_accel(&acc);
    imu.get_gyro(&gyr);
    imu.get_magnet(&mag);

    values[0] = acc.x;
    values[1] = acc.y;
    values[2] = acc.z;

    values[3] = mag.x;
    values[4] = mag.y;
    values[5] = mag.z;

    values[6] = gyr.x;
    values[7] = gyr.y;
    values[8] = gyr.z;

    #endif
    return;
}

void imu_read(int axis)
{
    char buf[32];
    memset(buf,0,sizeof(buf));

    #ifdef IMU_IS_BMX055
    BMX055_ACCEL_TypeDef  acc;
    BMX055_GYRO_TypeDef   gyr;
    BMX055_MAGNET_TypeDef mag;
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
    float mx=0.0f,my=0.0f, mz=0.0f,temp=0.0f;
    #endif

    switch (axis)
    {
        case IMU_MAG:
            #ifdef IMU_IS_LSM9DS1
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_magnet(&mag);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", mag.x,  mag.y, mag.z);
            #endif
            #ifdef IMU_IS_ICM_AND_MMC
            read_mmc5633(&mx, &my, &mz, &temp);
            printf("Temp: %6.4lf, x: %6.4lf, y: %6.4lf, z: %6.4lf\r\n", temp, mx, my, mz);
            #endif
            return;
        case IMU_GYR:
            #ifdef IMU_IS_LSM9DS1
            imu.readGyro();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz));
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_gyro(&gyr);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", gyr.x,  gyr.y, gyr.z);
            #endif
            return;
        case IMU_ACC:
            #ifdef IMU_IS_LSM9DS1
            imu.readAccel();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az));
            /////TODO: check data length and send packet from other cases as well
            float values[] = { imu.calcAccel(imu.ax),  imu.calcAccel(imu.ay), imu.calcAccel(imu.az) };
            sendPacket(IMU, acc_all, buf, 12);
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_accel(&acc);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", acc.x,  acc.y, acc.z);
            /////TODO: check data length and send packet from other cases as well
            float values[] = { acc.x, acc.y, acc.z };
            sendPacket(IMU, acc_all, values, 12);
            #endif
            return;
        case IMU_CALIB:
            imu_calibrate();
            _debug("IMU Calibration succesful.\n\r");
            break;
        case IMU_ALL:
        default:
            #ifdef IMU_IS_LSM9DS1
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            _debug("Accel: %s\n\r", buf);
            imu.readGyro();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz));
            _debug("Gyro: %s\n\r", buf);
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            _debug("Mag: %s\n\r", buf);
            #endif

            char floatBuf[36];
            memset(floatBuf, 0, sizeof(floatBuf));

            #ifdef IMU_IS_LSM9DS1
            imu.readAccel();
            imu.readGyro();
            imu.readMag();

            float values[] = {imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz), 
                imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz),
                imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az)
             };

             for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_accel(&acc);
            imu.get_gyro(&gyr);
            imu.get_magnet(&mag);
            float values[] = { mag.x,  mag.y,  mag.z, 
               gyr.x, gyr.y, gyr.z,
                acc.x, acc.y, acc.z
             };

             for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            #endif
            #ifdef IMU_IS_ICM_AND_MMC
            float values[] = { 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f};

            read_mmc5633(&mx, &my, &mz, &temp);
            values[0] = mx; values[1] = my; values[2] = mz;
            imu.readData(&values[3]);

            for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            
            #endif
            
            
            sendPacket(IMU, 14, floatBuf, 36);
            break;
    }
}

void imu_read_tobuffer(int axis, float* floatBuf)
{
    char buf[32];
    memset(buf,0,sizeof(buf));

    #ifdef IMU_IS_BMX055
    BMX055_ACCEL_TypeDef  acc;
    BMX055_GYRO_TypeDef   gyr;
    BMX055_MAGNET_TypeDef mag;
    #endif
    #ifdef IMU_IS_ICM_AND_MMC
    float mx=0.0f,my=0.0f, mz=0.0f,temp=0.0f;
    #endif

    switch (axis)
    {
        case IMU_MAG:
            #ifdef IMU_IS_LSM9DS1
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_magnet(&mag);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", mag.x,  mag.y, mag.z);
            #endif
            #ifdef IMU_IS_ICM_AND_MMC
            read_mmc5633(&mx, &my, &mz, &temp);
            printf("Temp: %6.4lf, x: %6.4lf, y: %6.4lf, z: %6.4lf\r\n", temp, mx, my, mz);
            #endif
            return;
        case IMU_GYR:
            #ifdef IMU_IS_LSM9DS1
            imu.readGyro();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz));
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_gyro(&gyr);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", gyr.x,  gyr.y, gyr.z);
            #endif
            return;
        case IMU_ACC:
            #ifdef IMU_IS_LSM9DS1
            imu.readAccel();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az));
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_accel(&acc);
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", acc.x,  acc.y, acc.z);
            /////TODO: send packets from these as well
            #endif
            return;
        case IMU_CALIB:
            imu_calibrate();
            _debug("IMU Calibration succesful.\n\r");
            break;
        case IMU_ALL:
        default:
            #ifdef IMU_IS_LSM9DS1
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            _debug("Accel: %s\n\r", buf);
            imu.readGyro();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz));
            _debug("Gyro: %s\n\r", buf);
            imu.readMag();
            sprintf((char*)buf, "%4.2f,%4.2f,%4.2f", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
            _debug("Mag: %s\n\r", buf);
            #endif

            #ifdef IMU_IS_LSM9DS1
            imu.readAccel();
            imu.readGyro();
            imu.readMag();

            float values[] = {imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz), 
                imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz),
                imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az)
             };

             for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            #endif
            #ifdef IMU_IS_BMX055
            imu.get_accel(&acc);
            imu.get_gyro(&gyr);
            imu.get_magnet(&mag);
            float values[] = { mag.x,  mag.y,  mag.z, 
               gyr.x, gyr.y, gyr.z,
                acc.x, acc.y, acc.z
             };

             for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            #endif
            #ifdef IMU_IS_ICM_AND_MMC
            float values[] = { 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f};

            read_mmc5633(&mx, &my, &mz, &temp);
            values[0] = mx; values[1] = my; values[2] = mz;
            imu.readData(&values[3]);

            for(int i=0; i<9; i++)
            {
                memcpy(floatBuf+(i*4), values+i, 4);
            }
            #endif
            
            break;
    }
}
*/
