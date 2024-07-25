#include "ICM-20608-G.h"
#include "ICM-20608-G_registers.h"
#include "../../obc/wd.h"



ICM_20608_G::ICM_20608_G(GyroRange g, AccelRange a, PinName sda, PinName scl) : gyro_range(g), accel_range(a), acc_gyr_i2c(sda, scl)
{
    switch (gyro_range)
    {
    case G_250_DEGS:
        gyro_lsb_to_degs = 250.f / 32768.f;
        break;
    case G_500_DEGS:
        gyro_lsb_to_degs = 500.f / 32768.f;
        break;
    case G_1000_DEGS:
        gyro_lsb_to_degs = 1000.f / 32768.f;
        break;
    case G_2000_DEGS:
        gyro_lsb_to_degs = 2000.f / 32768.f;
        break;
    }

    switch (accel_range)
    {
    case A_2_G:
        accel_lsb_to_g = 2.f / 32768.f;
        break;
    case A_4_G:
        accel_lsb_to_g = 4.f / 32768.f;
        break;
    case A_8_G:
        accel_lsb_to_g = 8.f / 32768.f;
        break;
    case A_16_G:
        accel_lsb_to_g = 16.f / 32768.f;
        break;
    }

    //_xgAddress = 0b1101000; // accgyro
    _xgAddress = 0b1101001 << 1;
    // alternatively: _xgAddress = 0b1101001;
}

int8_t ICM_20608_G::whoami()
{
    char who = 0;
    who = readRegister(_xgAddress, WHOAMI);

    return -1;
}

int8_t ICM_20608_G::init()
{
    //I2C acc_gyr_i2c(IMU_I2C_SDA, IMU_I2C_SCL);

    // try awaking the device 5 times
    uint8_t i;
    for (i = 1; i <= 5; i++)
    {
        writeRegister(_xgAddress, PWR_MGMT_1, RESET);     // reset whole device
        feed_the_dog();
        ThisThread::sleep_for(100ms);

        //writeRegister(_xgAddress, USER_CTRL, I2C_IF_EN); // disable I2C mode as recommended in datasheet
        writeRegister(_xgAddress, PWR_MGMT_1, CLK_ZGYRO); // wake up

        ThisThread::sleep_for(500ms);
        feed_the_dog();
        ThisThread::sleep_for(500ms);
        feed_the_dog();
        ThisThread::sleep_for(500ms);
        feed_the_dog();
        ThisThread::sleep_for(500ms);
        feed_the_dog();
        ThisThread::sleep_for(500ms);
        feed_the_dog();

        debug("Attempting to read pwr mgmt register with value zgyro...\n\r");

        // woke up or still asleep?
        if (readRegister(_xgAddress, PWR_MGMT_1) == CLK_ZGYRO)
        {
            // woke up, go on
            debug("Success...\n\r");
            break;
        }
    }

    if (i >= 5)
    {
        return -1;
    }

    writeRegister(_xgAddress, UNDOC1, UNDOC1_VALUE);

    switch (gyro_range)
    {
    case G_250_DEGS:
        writeRegister(_xgAddress, GYRO_CONFIG, DPS250);
        break;
    case G_500_DEGS:
        writeRegister(_xgAddress, GYRO_CONFIG, DPS500);
        break;
    case G_1000_DEGS:
        writeRegister(_xgAddress, GYRO_CONFIG, DPS1000);
        break;
    case G_2000_DEGS:
        writeRegister(_xgAddress, GYRO_CONFIG, DPS2000);
        break;
    }

    switch (accel_range)
    {
    case A_2_G:
        writeRegister(_xgAddress, ACCEL_CONFIG, G2);
        break;
    case A_4_G:
        writeRegister(_xgAddress, ACCEL_CONFIG, G4);
        break;
    case A_8_G:
        writeRegister(_xgAddress, ACCEL_CONFIG, G8);
        break;
    case A_16_G:
        writeRegister(_xgAddress, ACCEL_CONFIG, G16);
        break;
    }

    feed_the_dog();

    return 0;
}

void ICM_20608_G::calibrate()
{
    const uint16_t number_messurements = 500;

    int16_t gx, gy, gz;
    int16_t ax, ay, az;

    int32_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
    int32_t sum_ax = 0, sum_ay = 0, sum_az = 0;

    for (uint16_t i = 0; i < number_messurements; i++)
    {
        readGyroRaw(gx, gy, gz);
        readAccelRaw(ax, ay, az);

        sum_gx += gx;
        sum_gy += gy;
        sum_gz += gz;

        sum_ax += ax;
        sum_ay += ay;
        sum_az += az;

        //delay(5);
    }

    offset_gx = -sum_gx / number_messurements;
    offset_gy = -sum_gy / number_messurements;
    offset_gz = -sum_gz / number_messurements;
    offset_ax = -sum_ax / number_messurements;
    offset_ay = -sum_ay / number_messurements;
    offset_az = 1 / accel_lsb_to_g - sum_az / (float) number_messurements;
}

void ICM_20608_G::writeRegister(uint8_t address, uint8_t subAddress, uint8_t data)
{
    char temp_data[2] = {subAddress, data};
    acc_gyr_i2c.write(address, temp_data, 2);
    /*SPI.beginTransaction(settings);
    digitalWrite(CS, LOW);

    SPI.transfer(reg & ~(1 << 7)); // MSB = 0 for Writing
    SPI.transfer(val);

    digitalWrite(CS, HIGH);
    SPI.endTransaction();*/
}

uint8_t ICM_20608_G::readRegister(uint8_t address, uint8_t subAddress)
{
    char data = 0;
    // NOTE: increased array size from 1 to 2
    char temp[2] = {subAddress};
    
    acc_gyr_i2c.write(address, temp, 1);
    //i2c.write(address & 0xFE);
    temp[1] = 0x00;
    acc_gyr_i2c.write(address, temp, 1);
    //i2c.write( address | 0x01);
    int a = acc_gyr_i2c.read(address, &data, 1);

    return data;
    /*SPI.beginTransaction(settings);
    digitalWrite(CS, LOW);

    SPI.transfer(reg | (1 << 7)); // MSB = 1 for Reading
    uint8_t val = SPI.transfer(0);

    digitalWrite(CS, HIGH);
    SPI.endTransaction();*/
}

void ICM_20608_G::readGyroRaw(int16_t &x, int16_t &y, int16_t &z) const
{
    /*SPI.beginTransaction(settings);
    digitalWrite(CS, LOW);

    SPI.transfer(GYRO_XOUT_H | (1 << 7));
    x = ((int16_t)SPI.transfer(GYRO_XOUT_L | (1 << 7))) << 8;
    x |= SPI.transfer(GYRO_YOUT_H | (1 << 7));
    y = ((int16_t)SPI.transfer(GYRO_YOUT_L | (1 << 7))) << 8;
    y |= SPI.transfer(GYRO_ZOUT_H | (1 << 7));
    z = ((int16_t)SPI.transfer(GYRO_ZOUT_L | (1 << 7))) << 8;
    z |= SPI.transfer(0);

    digitalWrite(CS, HIGH);
    SPI.endTransaction();*/
}

void ICM_20608_G::readGyro(float &x, float &y, float &z)
{
    uint8_t temp[16];
    readRegisters(_xgAddress, GYRO_XOUT_H, temp, 6);
    int16_t rawax =     ((uint16_t)temp[0] << 8) | (uint16_t)temp[1];
    int16_t raway =     ((uint16_t)temp[2] << 8) | (uint16_t)temp[3];
    int16_t rawaz =     ((uint16_t)temp[4] << 8) | (uint16_t)temp[5];

    x = (float)(rawax + offset_ax) * accel_lsb_to_g;
    y = (float)(raway + offset_ay) * accel_lsb_to_g;
    z = (float)(rawaz + offset_az) * accel_lsb_to_g;
}



void ICM_20608_G::readRegisters(uint8_t address, uint8_t regAddress, uint8_t* dataBuffer, uint32_t numBytes)
{
    char cmd[32];
    memset(cmd, 0, 32);
    cmd[0] = regAddress;
    memcpy(cmd+1, dataBuffer, numBytes); 
    acc_gyr_i2c.write(address, cmd, 1);
    acc_gyr_i2c.read(address,cmd+1, numBytes);
    memcpy(dataBuffer, cmd+1, numBytes);
}


void ICM_20608_G::readData(float *buf)
{
    uint8_t temp[16];
    readRegisters(_xgAddress, 0x3b, temp, 14);
    int16_t rawax =     ((uint16_t)temp[0] << 8) | (uint16_t)temp[1];
    int16_t raway =     ((uint16_t)temp[2] << 8) | (uint16_t)temp[3];
    int16_t rawaz =     ((uint16_t)temp[4] << 8) | (uint16_t)temp[5];
    int16_t rawtemp =   ((uint16_t)temp[6] << 8) | (uint16_t)temp[7];
    int16_t rawgx =     ((uint16_t)temp[8] << 8) | (uint16_t)temp[9];
    int16_t rawgy =     ((uint16_t)temp[10] << 8) | (uint16_t)temp[11];
    int16_t rawgz =     ((uint16_t)temp[12] << 8) | (uint16_t)temp[13];

    float ax = (float)(rawax + offset_ax) * accel_lsb_to_g;
    float ay = (float)(raway + offset_ay) * accel_lsb_to_g;
    float az = (float)(rawaz + offset_az) * accel_lsb_to_g;
    float gx = (float)(rawgx + offset_gx) * gyro_lsb_to_degs;
    float gy = (float)(rawgy + offset_gy) * gyro_lsb_to_degs;
    float gz = (float)(rawgz + offset_gz) * gyro_lsb_to_degs;
    //float t = (float)(rawtemp + RoomTemp_Offset) * Temp_Sensitivity;

    //printf("ax: %5.2f, ay: %5.2f, az: %5.2f, gx: %5.2f, gy: %5.2f, gz: %5.2f\r\n", ax, ay, az, gx, gy, gz);

    buf[0] = gx;
    buf[1] = gy;
    buf[2] = gz;

    buf[3] = ax;
    buf[4] = ay;
    buf[5] = az;
}


void ICM_20608_G::readAccelRaw(int16_t &x, int16_t &y, int16_t &z) const
{
    /*SPI.beginTransaction(settings);
    digitalWrite(CS, LOW);

    SPI.transfer(ACCEL_XOUT_H | (1 << 7));
    x = ((int16_t)SPI.transfer(ACCEL_XOUT_L | (1 << 7))) << 8;
    x |= SPI.transfer(ACCEL_YOUT_H | (1 << 7));
    y = ((int16_t)SPI.transfer(ACCEL_YOUT_L | (1 << 7))) << 8;
    y |= SPI.transfer(ACCEL_ZOUT_H | (1 << 7));
    z = ((int16_t)SPI.transfer(ACCEL_ZOUT_L | (1 << 7))) << 8;
    z |= SPI.transfer(0);

    digitalWrite(CS, HIGH);
    SPI.endTransaction();*/
}

void ICM_20608_G::readAccel(float &x, float &y, float &z)
{
    uint8_t temp[16];
    readRegisters(_xgAddress, ACCEL_XOUT_H, temp, 6);
    int16_t rawax =     ((uint16_t)temp[0] << 8) | (uint16_t)temp[1];
    int16_t raway =     ((uint16_t)temp[2] << 8) | (uint16_t)temp[3];
    int16_t rawaz =     ((uint16_t)temp[4] << 8) | (uint16_t)temp[5];

    x = (float)(rawax + offset_ax) * accel_lsb_to_g;
    y = (float)(raway + offset_ay) * accel_lsb_to_g;
    z = (float)(rawaz + offset_az) * accel_lsb_to_g;
    //float t = (float)(rawtemp + RoomTemp_Offset) * Temp_Sensitivity;

    //printf("ax: %5.2f, ay: %5.2f, az: %5.2f, gx: %5.2f, gy: %5.2f, gz: %5.2f\r\n", ax, ay, az, gx, gy, gz);
}