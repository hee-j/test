#include "DPS310.h"
#include "dps310_config.h"
#include <cstdint>
#include <stdint.h>

extern I2C i2c2;
const int32_t DPS310::scaling_facts[DPS__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

char dps_address = 0x77 << 1;
#define address dps_address
DPS310 mdps(&i2c2, address);
void setup_pres();
void setup_temp();
//void setup_fifo();
float final_temp_value(char res1, char res2, char res3);
void read_res(char *mregister, char* reslut);
float final_pres_value(char res1, char res2, char res3);

void read_registers(char mregister, char *data, int datalen);
int twoscomplement(int val, uint8_t base);
void read_all_coeffs();

int m_c0Half;
int m_c1;
int m_c00;
int m_c10;
int m_c01;
int m_c11;
int m_c20;
int m_c21;
int m_c30;

float temp_raw_latest;

#define _TEMP_MODE            1
#define _PRES_MODE            2
//#define DPS310_FIFO_ON       1

int dps_sensor_init(void)
{
    
    i2c2.frequency(10000);
    uint8_t init_res = mdps.init(&i2c2, address);
    
    setup_temp();
    setup_pres();
    //setup_fifo();
    read_all_coeffs();
    #ifdef DPS310_FIFO_ON
        char fiforeg[1];
        fiforeg[0] = 0x0B;
        char fiforeslut[2];
        fiforeslut[0] = 0x00;
        fiforeslut[1] = 0x00;
    #endif

    return init_res;
}


void setup_meas_ctrl(int MODE) {
    char meas_cfg[2];
    meas_cfg[0] = 0x08;

    // TEMP on
    if(MODE == _TEMP_MODE) {
        meas_cfg[1] = 0b00000010;
    }
    else
        meas_cfg[1] = 0b00000001;

    i2c2.write(address, meas_cfg, 2);
}

float dps_sensor_pressure(void)
{
    
    char reslut[3];
    setup_meas_ctrl(_PRES_MODE);
    read_registers(0x00, reslut, 3);



    float pres_b        = (float)final_pres_value(reslut[0], reslut[1], reslut[2]);
    return pres_b*0.01;
}


int compensation_scale_factor_temp()
{
    int factor = 7864320;
    return factor;
}

int compensation_scale_factor_pres()
{
    int factor = 7864320;
    return factor;
}


void read_registers(char mregister, char *data, int datalen)
{
    i2c2.write(address, &mregister, 1);
    i2c2.read(address, (char*) data, datalen);
}

void read_res(char *mregister, char* reslut)
{
    i2c2.write(address, mregister, 1);
    i2c2.read(address, (char*) reslut, 1);
}


float dps_sensor_temperature(void)
{
    
    char reslut[3];
    float temperature = 0.0f;
    setup_meas_ctrl(_TEMP_MODE);

    read_registers(0x03, reslut, 3);

    return final_temp_value(reslut[2], reslut[1], reslut[0]);
    
    /*
    float res;
    mdps.measureTempOnce(&res);
    return res;*/
}


float final_temp_value(char res1, char res2, char res3)
{
    uint32_t t_raw = res1 | (res2<<8) | (res3<<16);
    //printf("Lämpötilatietueiden yhdistelmä: %08X\n\r", t_raw);
    float t_raw_float = 1.0 * t_raw;
    float t_raw_sc = ((float) t_raw_float) / compensation_scale_factor_temp();
    temp_raw_latest = t_raw_sc;
    
    float temp = t_raw_sc;
    temp = m_c0Half + m_c1 * temp;

    #ifdef DPS310_FIFO_ON
    flush_fifo();
    #endif

    return temp;//*0.1693;
}


float final_pres_value(char res1, char res2, char res3)
{
    int p_raw;
    p_raw = (uint32_t) res1 << 16 | (uint32_t)res2 << 8 | (uint32_t)res3;

    p_raw = twoscomplement(p_raw, 24);
    float p_raw_float = 1.0 * p_raw;
    float p_raw_sc = ((float) p_raw_float) / compensation_scale_factor_pres();

    printf("Raw pres: %.2f\n\r", (float)p_raw_sc);

//    read_all_coeffs();

    float prs = p_raw_sc;

    prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + temp_raw_latest * (m_c01 + prs * (m_c11 + prs * m_c21));

    #ifdef DPS310_FIFO_ON
    flush_fifo();
    #endif

    return prs;
}

int twoscomplement(int val, uint8_t base)
{
    switch (base) {
        case 12:
            if(val>2047) return val-4096;
            return val;
        case 16:
            if(val>32767) return val-65536;
            return val;
        case 20:
            if(val>524287) return val-1048576;
            return val;
        case 24:
            if(val>8388607) return val-16777216;
            return val;
        default:
        return 0;
    }
}

void read_temp_calibration_coefficients(int* c0, int* c1)
{
    char coeffs[3];
    read_registers(0x10, coeffs, 3);

    char c0reg = coeffs[0];
    char c01reg = coeffs[1];
    char c1reg = coeffs[2];

    *c0 = ( ((c01reg >> 4) & 0x0F) | ((c0reg) << 4 ));
    *c1 = (((c1reg)) | (  (c01reg & 0x0F) << 8 ));
    *c0 = twoscomplement(*c0, 12);
    *c1 = twoscomplement(*c1, 12);
    
}


void read_all_coeffs() {

    char results[18];
    read_registers(0x10, results, 18);

    //compose coefficients from buffer content
	m_c0Half = ((uint32_t)results[0] << 4) | (((uint32_t)results[1] >> 4) & 0x0F);
	//mgetTwosComplement((int32_t*)&m_c0Half, 12);
	//c0 is only used as c0*0.5, so c0_half is calculated immediately
    m_c0Half = m_c0Half / 2U;

    //now do the same thing for all other coefficients
	m_c1 = (((uint32_t)results[1] & 0x0F) << 8) | (uint32_t)results[2];
    m_c1 = twoscomplement(m_c1, 12);
	//mgetTwosComplement((int32_t*)&m_c1, 12);
	m_c00 = ((uint32_t)results[3] << 12) | ((uint32_t)results[4] << 4) | (((uint32_t)results[5] >> 4) & 0x0F);
	m_c00  = twoscomplement(m_c00, 20);
    //mgetTwosComplement((int32_t*)&m_c00, 20);
	m_c10 = (((uint32_t)results[5] & 0x0F) << 16) | ((uint32_t)results[6] << 8) | (uint32_t)results[7];
	m_c10 = twoscomplement(m_c10, 20);
    //mgetTwosComplement((int32_t*)&m_c10, 20);

	m_c01 = ((uint32_t)results[8] << 8) | (uint32_t)results[9];
    m_c01 = twoscomplement(m_c01, 16);
	//mgetTwosComplement((int32_t*)&m_c01, 16);

	m_c11 = ((uint32_t)results[10] << 8) | (uint32_t)results[11];
    m_c11 = twoscomplement(m_c11, 16);
	//mgetTwosComplement((int32_t*)&m_c11, 16);
	m_c20 = ((uint32_t)results[12] << 8) | (uint32_t)results[13];
    m_c20 = twoscomplement(m_c20, 16);
	//mgetTwosComplement((int32_t*)&m_c20, 16);
	m_c21 = ((uint32_t)results[14] << 8) | (uint32_t)results[15];
	//mgetTwosComplement((int32_t*)&m_c21, 16);
    m_c21 = twoscomplement(m_c21, 16);
	m_c30 = ((uint32_t)results[16] << 8) | (uint32_t)results[17];
    m_c30 = twoscomplement(m_c30, 16);
	//mgetTwosComplement((int32_t*)&m_c30, 16);
}



/*
void setup_fifo()
{
    char cfgreg[2];
    cfgreg[0] = 0x09;

    #ifdef DPS310_FIFO_ON
    cfgreg[1] = 0b00110010;
    #else
    cfgreg[1] = 0b00110000;
    #endif
    i2c2.write(address, cfgreg, 2);
}
*/

void read_coef_src(char* reslut)
{
    char coef_reg = 0x28;
    // Read res
    i2c2.write(address, &coef_reg, 1);
    i2c2.read(address, (char*) reslut, 1);
}

void setup_temp()
{
    char reslut = 0;
    char tmpcgf[2];
    tmpcgf[0] = 0x07;
    tmpcgf[1] = 0b00010011; // Precision: 2, os rate: 8 meas/second
    
    read_coef_src(&reslut);

    printf("Source for temp coefficient is based on: %02X\n\r", reslut);

    if(reslut & 0b10000000)
    {
        printf("Using external sensor\n\r");
        tmpcgf[1] = 0b10010011; // 2 meas. / default sample rate
    }

    i2c2.write(address, tmpcgf, 2);
}

void setup_pres()
{
    char prescgf[2];
    prescgf[0] = 0x06;
    prescgf[1] = 0b00010011; // Precision: 2, rate: 8 meas/second
    i2c2.write(address, prescgf, 2);
}


DPS310::DPS310(I2C *i2c_bus, uint8_t slaveaddr)
{
    m_i2cbus = i2c_bus;
    //DPS310::init(i2c_bus, slaveaddr);
}

uint8_t DPS310::init(I2C *i2c_bus, uint8_t slaveaddr) {
    
    int16_t prodId = DPS310::readByteBitfield(dps310::registers[dps310::PROD_ID]);
	if (prodId < 0)
	{
		//Connected device is not a Dps310
        printf("Connected device is not dps...\n\r");
		m_initFail = 1U;
		return 1;
	}
	m_productID = prodId;


	int16_t revId = DPS310::readByteBitfield(dps310::registers[dps310::REV_ID]);
	if (revId < 0)
	{
		m_initFail = 1U;
		return 1;
	}
	m_revisionID = revId;

	//find out which temperature sensor is calibrated with coefficients...
	int16_t sensor = DPS310::readByteBitfield(dps310::registers[dps310::TEMP_SENSORREC]);
	if (sensor < 0)
	{
		m_initFail = 1U;
		return 1;
	}

	//...and use this sensor for temperature measurement
	m_tempSensor = sensor;
	if (writeByteBitfield((uint8_t)sensor, dps310::registers[dps310::TEMP_SENSOR]) < 0)
	{
		m_initFail = 1U;
		return 1;
	}

	//set to standby for further configuration
	DPS310::standby();

    
	//set measurement precision and rate to standard values;
	configTemp(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
    
	configPressure(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);


	//perform a first temperature measurement
	//the most recent temperature will be saved internally
	//and used for compensation when calculating pressure
    dps_sensor_temperature();

	//make sure the DPS310 is in standby after initialization
	DPS310::standby();

	// Fix IC with a fuse bit problem, which lead to a wrong temperature
	// Should not affect ICs without this problem
	DPS310::correctTemp();

    return 0;
}

int16_t DPS310::readByteBitfield(RegMask_t regMask)
{
	int16_t ret = DPS310::readByte(regMask.regAddress);
	if (ret < 0)
	{
		return ret;
	}
	return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}

int16_t DPS310::setOpMode(uint8_t opMode)
{
	if (writeByteBitfield(opMode, dps::config_registers[dps::MSR_CTRL]) == -1)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_opMode = (dps::Mode)opMode;
	return DPS__SUCCEEDED;
}

int16_t DPS310::readByte(uint8_t regAddress)
{
    char reslut[2];
    reslut[0] = 0x01;
    reslut[1] = 0x00;

    //m_i2cbus->write(m_slaveAddress, (char*) &nwaddr, 1);
    //return m_i2cbus->read(m_slaveAddress, (char*) &nwaddr, 1);

    m_i2cbus->write(m_slaveAddress, (char*) &regAddress, 1);
    m_i2cbus->read(m_slaveAddress, (char*) reslut, 1);

    return reslut[0];

}
/*
float DPS310::calcPressure(int32_t raw)
{
	float prs = raw;

	//scale pressure according to scaling table and oversampling
	prs /= scaling_facts[m_prsOsr];

	//Calculate compensated pressure
	prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));

	//return pressure
	return prs;
}
float DPS310::calcTemp(int32_t raw)
{
	float temp = raw;

	//scale temperature according to scaling table and oversampling
	temp /= scaling_facts[m_tempOsr];

	//update last measured temperature
	//it will be used for pressure compensation
	m_lastTempScal = temp;

	//Calculate compensated temperature
	temp = m_c0Half + m_c1 * temp;

	return temp;
}

int16_t DPS310::getRawResult(int32_t *raw, RegBlock_t reg)
{
	uint8_t buffer[DPS__RESULT_BLOCK_LENGTH] = {0};
	if (DPS310::readBlock(reg, buffer) != DPS__RESULT_BLOCK_LENGTH) {
        printf("Can't get raw result with readblock, %d\n\r", DPS__RESULT_BLOCK_LENGTH);
		return DPS__FAIL_UNKNOWN;
    }

    printf("Computing raw value...\n\r");
	*raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
	DPS310::getTwosComplement(raw, 24);
	return DPS__SUCCEEDED;
}

int16_t DPS310::getSingleResult(float &result)
{
	//abort if initialization failed
	if (m_initFail)
	{
        
		return DPS__FAIL_INIT_FAILED;
	}

	//read finished bit for current opMode
	int16_t rdy;
	switch (m_opMode)
	{
	case dps::CMD_TEMP: //temperature
        printf("Measuring temp.\n\r");
		rdy = DPS310::readByteBitfield(dps::config_registers[dps::TEMP_RDY]);
		break;
	case dps::CMD_PRS: //pressure
		rdy = DPS310::readByteBitfield(dps::config_registers[dps::PRS_RDY]);
		break;
	default: //DPS310 not in command mode
		return DPS__FAIL_TOOBUSY;
	}
	//read new measurement result

	switch (rdy)
	{
	case DPS__FAIL_UNKNOWN: //could not read ready flag
		return DPS__FAIL_UNKNOWN;
	case 0: //ready flag not set, measurement still in progress
        printf("Temp measurement not done! %d\n\r", DPS__FAIL_UNFINISHED);
	case 1: //measurement ready, expected case
        printf("Temp measurement expected case.\n\r");
		dps::Mode oldMode = m_opMode;
		m_opMode = dps::IDLE; //opcode was automatically reseted by DPS310
		int32_t raw_val;
		switch (oldMode)
		{
		case dps::CMD_TEMP: //temperature
			getRawResult(&raw_val, dps::registerBlocks[dps::TEMP]);
			result = calcTemp(raw_val);
			return DPS__SUCCEEDED; // TODO
		case dps::CMD_PRS:			   //pressure
			getRawResult(&raw_val, dps::registerBlocks[dps::PRS]);
			result = calcPressure(raw_val);
			return DPS__SUCCEEDED; // TODO
		default:
            printf("Fail.\n\r");
			return DPS__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS__FAIL_UNKNOWN;
}

int16_t DPS310::readcoeffs(void)
{
	// TODO: remove magic number
	uint8_t buffer[18];
	//read COEF registers to buffer
	int16_t ret = readBlock(dps310::coeffBlock, buffer);

	//compose coefficients from buffer content
	m_c0Half = ((uint32_t)buffer[0] << 4) | (((uint32_t)buffer[1] >> 4) & 0x0F);
	getTwosComplement(&m_c0Half, 12);
	//c0 is only used as c0*0.5, so c0_half is calculated immediately
	m_c0Half = m_c0Half / 2U;

	//now do the same thing for all other coefficients
	m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
	getTwosComplement(&m_c1, 12);
	m_c00 = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (((uint32_t)buffer[5] >> 4) & 0x0F);
	getTwosComplement(&m_c00, 20);
	m_c10 = (((uint32_t)buffer[5] & 0x0F) << 16) | ((uint32_t)buffer[6] << 8) | (uint32_t)buffer[7];
	getTwosComplement(&m_c10, 20);

	m_c01 = ((uint32_t)buffer[8] << 8) | (uint32_t)buffer[9];
	getTwosComplement(&m_c01, 16);

	m_c11 = ((uint32_t)buffer[10] << 8) | (uint32_t)buffer[11];
	getTwosComplement(&m_c11, 16);
	m_c20 = ((uint32_t)buffer[12] << 8) | (uint32_t)buffer[13];
	getTwosComplement(&m_c20, 16);
	m_c21 = ((uint32_t)buffer[14] << 8) | (uint32_t)buffer[15];
	getTwosComplement(&m_c21, 16);
	m_c30 = ((uint32_t)buffer[16] << 8) | (uint32_t)buffer[17];
	getTwosComplement(&m_c30, 16);
	return DPS__SUCCEEDED;
}
*/
int16_t DPS310::disableFIFO()
{
	int16_t ret = flushFIFO();
	ret = writeByteBitfield(0U, dps::config_registers[dps::FIFO_EN]);
	return ret;
}


int16_t DPS310::flushFIFO()
{
	return writeByteBitfield(1U, dps310::registers[dps310::FIFO_FL]);
}
/*
void DPS310::getTwosComplement(int32_t *raw, uint8_t length)
{
	if (*raw & ((uint32_t)1 << (length - 1)))
	{
		*raw -= (uint32_t)1 << length;
	}
}
*/
int16_t DPS310::configPressure(uint8_t prsMr, uint8_t prsOsr)
{
	int16_t ret = 0; //DPS310::configPressure(prsMr, prsOsr);
	//set PM SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (prsOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, dps310::registers[dps310::PRS_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, dps310::registers[dps310::PRS_SE]);
	}
	return ret;
}
/*
int16_t DPS310::measureTempOnce(float *result)
{
	return DPS310::measureTempOnce(result, m_tempOsr);
}

int16_t DPS310::measureTempOnce(float *result, uint8_t oversamplingRate)
{
	//Start measurement
	int16_t ret = DPS310::startMeasureTempOnce(oversamplingRate);
	if (ret != DPS__SUCCEEDED)
	{
        printf("Everything failed\n\r");
		return ret;
	}

	//wait until measurement is finished
	wait_us((DPS310::calcBusyTime(0U, m_tempOsr) / DPS__BUSYTIME_SCALING)*1000);
	wait_us((DPS310__BUSYTIME_FAILSAFE)*1000);
	ret = getSingleResult(*result);
    
	if (ret != DPS__SUCCEEDED)
	{
		standby();
	}

    return ret;
}


int16_t DPS310::startMeasureTempOnce(void)
{
	return startMeasureTempOnce(m_tempOsr);
}

int16_t DPS310::startMeasureTempOnce(uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
        printf("Init failure\n\r");
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != dps::IDLE)
	{
        printf("Too busy\n\r");
		return DPS__FAIL_TOOBUSY;
	}

	if (oversamplingRate != m_tempOsr)
	{
		//configuration of oversampling rate
		if (configTemp(0U, oversamplingRate) != DPS__SUCCEEDED)
		{
            printf("Unknown failure\n\r");
			return DPS__FAIL_UNKNOWN;
		}
        printf("Unknown failure...\n\r");
	}

	//set device to temperature measuring mode
	return setOpMode(dps::CMD_TEMP);
}

uint16_t DPS310::calcBusyTime(uint16_t mr, uint16_t osr)
{
	//formula from datasheet (optimized)
	return ((uint32_t)20U << mr) + ((uint32_t)16U << (osr + mr));
}
*/
int16_t DPS310::configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	int16_t ret = DPS310::configTemp2(tempMr, tempOsr);

	writeByteBitfield(m_tempSensor, dps310::registers[dps310::TEMP_SENSOR]);

	//set TEMP SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (tempOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, dps310::registers[dps310::TEMP_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, dps310::registers[dps310::TEMP_SE]);
	}

    return ret;
}



int16_t DPS310::configTemp2(uint8_t tempMr, uint8_t tempOsr)
{
	tempMr &= 0x07;
	tempOsr &= 0x07;
	// two accesses to the same register; for readability
	int16_t ret = writeByteBitfield(tempMr, dps::config_registers[dps::TEMP_MR]);
	ret = writeByteBitfield(tempOsr, dps::config_registers[dps::TEMP_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_tempMr = tempMr;
	m_tempOsr = tempOsr;

    return 0;
}


int16_t DPS310::writeByteBitfield(uint8_t data, RegMask_t regMask)
{
	return DPS310::writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

int16_t DPS310::writeByteBitfield(uint8_t data,
									uint8_t regAddress,
									uint8_t mask,
									uint8_t shift,
									uint8_t check)
{
	int16_t old = readByte(regAddress);
	if (old < 0)
	{
		//fail while reading
		return old;
	}
	return DPS310::writeByte(regAddress, ((uint8_t)old & ~mask) | ((data << shift) & mask), check);
    return 0;
}
/*
int16_t DPS310::readBlock(RegBlock_t regBlock, uint8_t *buffer)
{
	//do not read if there is no buffer
	if (buffer == NULL)
	{
        return 0; //0 bytes read successfully
	}

    // ei jaza
    int16_t ret = 0;
    char reslut[2];
    reslut[0] = 0x00;
    reslut[1] = 0x00;

    int16_t count = 0;

    for (count = 0; count < regBlock.length; count++)
	{
        m_i2cbus->write(m_slaveAddress, (char*) &(regBlock.regAddress)+count, 1);
        m_i2cbus->read(m_slaveAddress, (char*) reslut, 1);
		buffer[count] = reslut[0]; // m_i2cbus->read(1);
	}
    ret = count;
	return ret;
}
*/
int16_t DPS310::writeByte(uint8_t regAddress, uint8_t data)
{
	return DPS310::writeByte(regAddress, data, 0U);
}
int16_t DPS310::writeByte(uint8_t regAddress, uint8_t data, uint8_t check)
{
	//m_i2cbus->beginTransmission(m_slaveAddress);

    //m_i2cbus->write(regAddress, (const char*)&data, 1);

    char out[2];
    out[0] = regAddress;
    out[1] = data;
    m_i2cbus->write(m_slaveAddress, out, 2);

    if (check == 0)
		return 0;					  //no checking
    if (readByte(regAddress) == data) //check if desired by calling function
    {
        return DPS__SUCCEEDED;
    }
    else
    {
        return DPS__FAIL_UNKNOWN;
    }

}

int16_t DPS310::correctTemp(void)
{
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	writeByte(0x0E, 0xA5);
	writeByte(0x0F, 0x96);
	writeByte(0x62, 0x02);
	writeByte(0x0E, 0x00);
	writeByte(0x0F, 0x00);

	//perform a first temperature measurement (again)
	//the most recent temperature will be saved internally
	//and used for compensation when calculating pressure
    dps_sensor_pressure();

	return DPS__SUCCEEDED;
}

int16_t DPS310::standby(void)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//set device to idling mode
	int16_t ret = setOpMode(dps::IDLE);
	if (ret != DPS__SUCCEEDED)
	{
		return ret;
	}
	ret = disableFIFO();
	return ret;
}