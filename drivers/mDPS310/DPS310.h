#ifndef DPS310_H_INCLUDED
#define DPS310_H_INCLUDED

#include <mbed.h>
#include "dps310_config.h"
#include "dps_config.h"




int dps_sensor_init(void);
float dps_sensor_temperature(void);
float dps_sensor_pressure(void);

class DPS310
{
public:
  int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount);
  int16_t setInterruptSources(uint8_t intr_source, uint8_t polarity = 1);

  DPS310(I2C *i2c_bus, uint8_t slaveAddress);

    /**
        * I2C begin function with standard address
        */
    //void begin(I2C &bus);

    /**
        * Standard I2C begin function
        *
        * @param &bus: 			I2CBus which connects MC to the sensor
        * @param slaveAddress: 	I2C address of the sensor (0x77 or 0x76)
        */
  uint8_t init(I2C *i2c_bus, uint8_t slaveaddr);
    void begin(I2C &bus, uint8_t slaveAddress);
    int16_t measurePressureOnce(float &result);
    int16_t measurePressureOnce(float &result, uint8_t oversamplingRate);
    int16_t startMeasurePressureOnce(void);
    int16_t startMeasurePressureOnce(uint8_t oversamplingRate);
    int16_t startMeasureTempOnce(void);
    int16_t startMeasureTempOnce(uint8_t oversamplingRate);
    int16_t getSingleResult(float &result);
    int16_t correctTemp(void);
    int16_t standby(void);

    int16_t measureTempOnce(float *result);
	int16_t measureTempOnce(float *result, uint8_t oversamplingRate);

      void getTwosComplement(int32_t *raw, uint8_t length);


protected:
  uint8_t m_tempSensor;

  //compensation coefficients
  int32_t m_c0Half;
  int32_t m_c1;
  static const int32_t scaling_facts[DPS__NUM_OF_SCAL_FACTS];
	dps::Mode m_opMode;
	//flags
	uint8_t m_initFail;
	uint8_t m_productID;
	uint8_t m_revisionID;
	//settings
	uint8_t m_tempMr;
	uint8_t m_tempOsr;
	uint8_t m_prsMr;
	uint8_t m_prsOsr;
	// compensation coefficients for both dps310 and dps422
	int32_t m_c00;
	int32_t m_c10;
	int32_t m_c01;
	int32_t m_c11;
	int32_t m_c20;
	int32_t m_c21;
	int32_t m_c30;
	// last measured scaled temperature (necessary for pressure compensation)
	float m_lastTempScal;

  uint16_t calcBusyTime(uint16_t temp_rate, uint16_t temp_osr);

  /////// implement pure virtual functions ///////

  int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
  int16_t configTemp2(uint8_t tempMr, uint8_t tempOsr);
  int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);
  int16_t readcoeffs(void);
  int16_t flushFIFO();
  float calcTemp(int32_t raw);
  float calcPressure(int32_t raw);
  int16_t enableFIFO();
  int16_t disableFIFO();
  int16_t getFIFOvalue(int32_t *value);
  int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount, RegMask_t reg);


  int16_t setOpMode(uint8_t opMode);

  int16_t writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check);
  int16_t readByteBitfield(RegMask_t regMask);
  int16_t readByte(uint8_t regAddress);
  int16_t readBlock(RegBlock_t regBlock, uint8_t *buffer);
  int16_t writeByte(uint8_t regAddress, uint8_t data);
  int16_t writeByte(uint8_t regAddress, uint8_t data, uint8_t check);
  int16_t writeByteBitfield(uint8_t data, RegMask_t regMask);

  int16_t getRawResult(int32_t *raw, RegBlock_t reg);

	//bus specific
	uint8_t m_SpiI2c; //0=SPI, 1=I2C

	//used for I2C
	//TwoWire *m_i2cbus;
    I2C     *m_i2cbus;
	uint8_t m_slaveAddress;
};

#endif