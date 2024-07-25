#include "mmc5633njl.h"
#include "../hw_config.h"
#include "obc/wd.h"
#include <cstdint>

#ifdef IMU_IS_ICM_AND_MMC

// THIS I2C IS ALREADY INITIATED!!
//I2C mmc_i2c(IMU_I2C_SDA, IMU_I2C_SCL);

//extern ICM_20608_G imu;

#define MMC5633NJL_INTERNAL_CONTROL_0 0x08

#define M_WRITE 0b01100000
#define M_READ 0b01100001


I2C mag_i2c(IMU_I2C_SDA, IMU_I2C_SCL);

uint8_t MMC5633NJL_ADDR = 0b0110000 << 1;
uint8_t MMC5633NJL_ADDR_READ = 0b0110001 << 1;

uint8_t mag_writeRegister(uint8_t address, uint8_t subAddress, uint8_t data)
{
    
    char temp_data0[2] = {subAddress, data};
    return mag_i2c.write(M_WRITE, temp_data0, 2);



    //char temp_data[3] = {M_WRITE, subAddress, data};
    //mag_i2c.write(address, temp_data, 3);

    // next: try communicating with separate writes
    //mag_i2c.write(address, temp_data, 2);
}


uint8_t mag_readRegisters(char *data, uint8_t address, uint8_t subAddress, uint8_t len)
{
    data[0] = subAddress;
    mag_i2c.write(M_WRITE, data, 1);
    //mag_i2c.write(M_WRITE, temp, 1);

    return mag_i2c.read(M_READ, data, len);
}


uint8_t mag_readRegister(uint8_t address, uint8_t subAddress)
{
    char data = 0;

    char temp[2] = {subAddress};
    mag_i2c.write(M_WRITE, temp, 1);
    //mag_i2c.write(M_WRITE, temp, 1);

    int reading = mag_i2c.read(M_READ, &data, 1);
    return data;
}

/*uint8_t mag_readRegister(uint8_t address, uint8_t subAddress)
{
    char data = 0;
    // NOTE: increased array size from 1 to 2
    char temp[2] = {subAddress};
    
    mag_i2c.write(address, temp, 1);
    //i2c.write(address & 0xFE);
    temp[1] = 0x00;
    mag_i2c.write(address, temp, 1);
    //i2c.write( address | 0x01);
    int a = mag_i2c.read(MMC5633NJL_ADDR_READ, &data, 1);

    return data;
}*/

int mmc5633_whoami(void)
{
    char who = 0;
    who = mag_readRegister(MMC5633NJL_ADDR, 0x39);
    debug("Product ID register value is: %02x\n\r", who);
    // oikein
    feed_the_dog();
    char status1 = 0;
    status1 = mag_readRegister(MMC5633NJL_ADDR, 0x18);
    debug("Status1 register value is: %02x\n\r", status1);

    return !(who == 0 && status1 == 0);
}

int setup_mmc5633() {

  return mag_writeRegister(MMC5633NJL_ADDR, 0x1B, 0x27);
  /*Wire.begin();
  Serial.begin(9600);
  delay(100);
  
  //// SET
  Wire.beginTransmission(MMC5883MA_ADDR); //Adress of I2C device
  Wire.write(MMC5883MA_INTERNAL_CONTROL_0);// I2C Register
  Wire.write(0x08);// Value
  Wire.endTransmission();

  // SET A TEMPERATURE MEASUREMENT
  Wire.beginTransmission(MMC5883MA_ADDR); //Adress of I2C device
  Wire.write(MMC5883MA_INTERNAL_CONTROL_0);// I2C Register
  Wire.write(0x02);// Value
  Wire.endTransmission();*/
}

uint32_t bytestoraw(char out1, char out2, char out3)
{
    uint32_t result = ((uint32_t)out1 << 12) | ((uint32_t)out2 << 4) | ((uint32_t)out3 >> 4);
    //printf("0x%02x, 0x%02x, 0x%02x -> %02x\r\n", out1, out2, out3, result);
    return result;
}

void read_mmc5633(float *x, float *y, float *z, float *temp) {
    char data[10];
    memset(data, 0, 10);
    mag_writeRegister(MMC5633NJL_ADDR, 0x1B, 0x02);
    ThisThread::sleep_for(1ms);
    mag_writeRegister(MMC5633NJL_ADDR, 0x1B, 0x01);

    ThisThread::sleep_for(10ms);
    mag_readRegisters(data, MMC5633NJL_ADDR, 0x00, 10);
 
    uint32_t raw_mag_x = bytestoraw(data[0], data[1], data[6]);
    uint32_t raw_mag_y = bytestoraw(data[2], data[3], data[7]);
    uint32_t raw_mag_z = bytestoraw(data[4], data[5], data[8]);
    uint8_t raw_temp = data[9];

    *temp = 0.8*raw_temp-75; // page 15
    *x = (1.0f*raw_mag_x - 524288)/16384;
    *y = (1.0f*raw_mag_y - 524288)/16384;
    *z = (1.0f*raw_mag_z - 524288)/16384;

    /*

    // Perform temp measurement
    mag_writeRegister(MMC5633NJL_ADDR, 0x1B, 0x02);

    // read temp
    char temp = 0;
    temp = mag_readRegister(MMC5633NJL_ADDR, 0x09);
    printf("Temp value is: %d\n\r", temp);

    // Perform mag measurement
    mag_writeRegister(MMC5633NJL_ADDR, 0x1B, 0x01);

    // read mag
    char magX0 = 0, magX1 = 0, magX2 = 0;
    magX0 = mag_readRegister(MMC5633NJL_ADDR, 0x00);
    magX1 = mag_readRegister(MMC5633NJL_ADDR, 0x01);
    magX2 = mag_readRegister(MMC5633NJL_ADDR, 0x06);
    printf("magx value is: %02x %02x %02x\n\r", magX0, magX1, magX2);

    char magY0 = 0, magY1 = 0, magY2 = 0;
    magY0 = mag_readRegister(MMC5633NJL_ADDR, 0x02);
    magY1 = mag_readRegister(MMC5633NJL_ADDR, 0x03);
    magY2 = mag_readRegister(MMC5633NJL_ADDR, 0x07);
    printf("magy value is: %02x %02x %02x\n\r", magY0, magY1, magY2);

    char magZ0 = 0, magZ1 = 0, magZ2 = 0;
    magZ0 = mag_readRegister(MMC5633NJL_ADDR, 0x04);
    magZ1 = mag_readRegister(MMC5633NJL_ADDR, 0x05);
    magZ2 = mag_readRegister(MMC5633NJL_ADDR, 0x08);
    printf("magz value is: %02x %02x %02x\n\r", magZ0, magZ1, magZ2);
*/
    
  /*Wire.beginTransmission(MMC5883MA_ADDR); //Adress of I2C device
  Wire.write(MMC5883MA_STATUS);// I2C Status Register
  Wire.requestFrom(MMC5883MA_ADDR, 1);
  iStatus = Wire.read(); //Status here is 127
  Wire.endTransmission(false);

  Wire.beginTransmission(MMC5883MA_ADDR); //Adress of I2C device
  Wire.write(MMC5883MA_TEMPERATURE);// I2C Register
  Wire.requestFrom(MMC5883MA_ADDR, 1); 
  temp = Wire.read(); //TEMP value is 18 (-62°C...)
  Wire.endTransmission(false);
  
  Serial.println(temp);*/
}


#endif