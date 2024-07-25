#ifndef __MMC5633_H
#define __MMC5633_H


    #define IMU_IS_ICM_AND_MMC 1
 
    #define MMC5633NJL_OUT 0x00
    #define MMC5633NJL_XOUT 0x00
    #define MMC5633NJL_XOUT_LOW 0x00
    #define MMC5633NJL_XOUT_HIGH 0x01
    #define MMC5633NJL_YOUT 0x02
    #define MMC5633NJL_YOUT_LOW 0x02
    #define MMC5633NJL_YOUT_HIGH 0x03
    #define MMC5633NJL_ZOUT 0x04
    #define MMC5633NJL_ZOUT_LOW 0x04
    #define MMC5633NJL_ZOUT_HIGH 0x05
    #define MMC5633NJL_TEMPERATURE 0x06
    #define MMC5633NJL_STATUS 0x07
    #define MMC5633NJL_INTERNAL_CONTROL_0 0x08
    #define MMC5633NJL_INTERNAL_CONTROL_1 0x09
    #define MMC5633NJL_INTERNAL_CONTROL_2 0x0A
    #define MMC5633NJL_X_THRESHOLD 0x0B
    #define MMC5633NJL_Y_THRESHOLD 0x0C
    #define MMC5633NJL_Z_THRESHOLD 0x0D
    #define MMC5633NJL_PRODUCT_ID 0x2F

    #define MMC5633NJL_DYNAMIC_RANGE 16
    #define MMC5633NJL_RESOLUTION 65536


    int setup_mmc5633(void);
    int mmc5633_whoami(void);
    void read_mmc5633(float *x, float *y, float *z, float *temp);

#endif