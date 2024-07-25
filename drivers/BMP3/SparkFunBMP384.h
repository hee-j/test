#ifndef __SPARKFUN_BMP384_H__
#define __SPARKFUN_BMP384_H__


#include "mbed.h"
#include "bmp3_api/bmp3.h"

// SparkFun's default I2C address is opposite of Bosch's default
#define BMP384_I2C_ADDRESS_DEFAULT BMP3_ADDR_I2C_SEC    // 0x77
#define BMP384_I2C_ADDRESS_SECONDARY BMP3_ADDR_I2C_PRIM // 0x76

int bmp3_sensor_init(void);
float bmp3_sensor_temperature(void);
float bmp3_sensor_pressure(void);

class BMP384
{
    public:
        // Constructor
        BMP384(char slave_adr);

        // Sensor initialization, must specify communication interface
        int8_t beginI2C(I2C &i2c_obj, char slave_adr);
       // int8_t beginSPI(uint8_t csPin, uint32_t clockFrequency = 100000);
        // Sensor initialization, after communication interface has been selected
        int8_t begin();

        // Configuration control, the begin functions will set defaults for these
        int8_t init();
        int8_t setMode(uint8_t mode);
        int8_t enablePressAndTemp(uint8_t pressEnable, uint8_t tempEnable);

        // Data acquisistion
        int8_t getSensorData(bmp3_data* data);
        int8_t getSensorStatus(bmp3_sens_status* sensorStatus);

        // Output data rate (ODR) control
        int8_t setODRFrequency(uint8_t odr);
        int8_t getODRFrequency(uint8_t* odr);

        // Oversampling (OSR) control
        int8_t setOSRMultipliers(bmp3_odr_filter_settings osrMultipliers);
        int8_t getOSRMultipliers(bmp3_odr_filter_settings* osrMultipliers);

        // IIR filter control
        int8_t setFilterCoefficient(uint8_t coefficient);

        // Interrupt control
        int8_t setInterruptSettings(bmp3_int_ctrl_settings interruptSettings);
        int8_t getInterruptStatus(bmp3_int_status* interruptStatus);

        // FIFO control
        int8_t setFIFOSettings(bmp3_fifo_settings fifoSettings);
        int8_t setFIFOWatermark(uint8_t numData);
        int8_t getFIFOLength(uint8_t* numData);
        int8_t getFIFOData(bmp3_data* data, uint8_t numData);
        int8_t flushFIFO();

    private:
        
        // Calculates the minimum ODR setting given
        uint8_t calculateMinODR(bmp3_odr_filter_settings osrMultipliers);
        
        // Helper function for computing log_2(x)
        float log2(float x);

        // Read/write helper functions
        static BMP3_INTF_RET_TYPE readRegisters(uint8_t regAddress, uint8_t* dataBuffer, uint32_t numBytes, void *nothing);
        static BMP3_INTF_RET_TYPE writeRegisters(uint8_t regAddress, const uint8_t* dataBuffer, uint32_t numBytes, void *nothing);

        // Delay helper function
        static void usDelay(uint32_t period, void *nothing);

        // Reference to the sensor
        struct bmp3_dev sensor;

};

#endif