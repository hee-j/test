#include "env.h"
#include "settings.h"
#include "hw_config.h"

#ifdef ENV_IS_BMP280
#include "BMP280/BMP280.h"
#endif
#ifdef ENV_IS_DPS310
#include "mDPS310/DPS310.h"
#include "mDPS310/dps310_config.h"
#endif
#ifdef ENV_IS_BMP384
#include "BMP3/SparkFunBMP384.h"
#endif

I2C i2c2(BMP_SDA, BMP_SCL);

int env_init(void)
{
    i2c2.frequency(10000);
    
    #ifdef ENV_IS_BMP280
    return bmp2_sensor_init();
    #endif
    #ifdef ENV_IS_DPS310
    return dps_sensor_init();
    #endif
    #ifdef ENV_IS_BMP384
    return bmp3_sensor_init();
    #endif
}

float env_get_temp()
{
    #ifdef ENV_IS_BMP280
    return bmp2_sensor_temperature();
    #endif
    #ifdef ENV_IS_DPS310
    return dps_sensor_temperature();
    #endif
    #ifdef ENV_IS_BMP384
    return bmp3_sensor_temperature();
    #endif
}

float env_get_pressure()
{
    #ifdef ENV_IS_BMP280
    return bmp2_sensor_pressure();
    #endif
    #ifdef ENV_IS_DPS310
    return dps_sensor_pressure();
    #endif
    #ifdef ENV_IS_BMP384
    return bmp3_sensor_pressure();
    #endif
}

void ENV_TEMP()
{
    float temperature = env_get_temp();
    char resp[9];
    memset(resp,0,sizeof(resp));
    sprintf(resp, "%5.2f", temperature);
    sendPacket(_BMP, 1, resp, strlen(resp));
}
void ENV_PRESS()
{
    char resp[9];
    memset(resp,0,sizeof(resp));
    float pressure = env_get_pressure();
    sprintf(resp, "%5.2f", pressure);
    sendPacket(_BMP, 2, resp, strlen(resp));
    
}

void ENV_ALL()
{
    char resp[20];
    float temperature = env_get_temp();
    float pressure = env_get_pressure();
    memset(resp,0,sizeof(resp));
    sprintf(resp, "%5.2f,%5.2f", pressure, temperature);
    sendPacket(_BMP, 3, resp, strlen(resp));
}