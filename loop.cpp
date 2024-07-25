#include "loop.h"
#include "obc/camera.h"
#include "obc/wd.h"
#include "utilities/m_util.h"
#include "obc/sdhc.h"
#include "settings.h"
#include "payload/gps.h"
#include "utilities/submodule.h"
#include "payload/radio/radio.h"
#include "payload/imu.h"
#include "eps/mcp.h"
#include "payload/radio/RadioController.h"
#include "payload/buzzer.h"
#include "utilities/serial/serial.h"
#include "mcp3008/mcp3008.h"
#include "utilities/test.h"
#include "payload/env.h"
#include "payload/led.h"
#include "external_payload.h"

//EventQueue hkp_queue;
//EventQueue tx_queue;

InterruptIn sw(PC_13);

char current_image_path[64];
char current_image_name_only[32];

// Runonce flags for testing
extern uint8_t __CAM_CAPTURE_FLAG;
extern uint8_t CAM_ENABLED;
extern uint8_t CAM_SHUTDOWN;
extern uint8_t CLOSE_STREAM;
extern uint8_t FLIGHT_MODE_ON;
extern uint8_t INTENSIVE_FLIGHT_MODE_ON;

//extern LSM9DS1 imu; // only if IMU_IS_LSM9DS1
extern MCP3008 mcp;
extern GePeS myGPS;

extern uint8_t PLS_READ_RADIO_DATA;

int time_of_last_image = 5000;
int time_of_last_meas = 50;
int time_of_last_beacon = 500;

#define INTENSIVE_IMAGE_INTERVAL    10000
#define DEFAULT_IMAGE_INTERVAL      30000
#define MEAS_INTERVAL               100
#define BEACON_INTERVAL             1000
#define LOG_FILE_PATH "/fs/mydir/data.log";

void log_meas();
void send_beacon();
void take_image();

void handle_cam_capture()
{
    if( CAM_ENABLED )
    {
        if( __CAM_CAPTURE_FLAG == 1 )
        {
            camera_capture_on();
            _debug("Capture started.\n\r");
            
            __CAM_CAPTURE_FLAG = 0; // Runonce
        }
        
        if( camera_save_initiated(current_image_name_only) )
        {
            _debug("File save initiated, exiting checkloop\n\r");
            CAM_ENABLED = 0;
        }
    }
}

void cleanup()
{
    // Unmount and deinit block device
    if( is_mounted())
        sd_cleanup();
}

void check_reset_timer()
{
  // Reset anyway in every 15 min
  if( ms_since_boot() > 900000)
  {
      _warning("Halting due to sw wd. This is normal.\n\r");
      cleanup();
      NVIC_SystemReset();
  }
}

void broadcast_beacon()
{
    // Nice beacon. Now do broadcast something useful, genius.
    std::string beacon = "KITSAT";
    //_debug("Broadcasting beacon: %s\n\r", beacon.c_str());
    
    //radio_tx((char*)beacon.c_str());
}

void manage_queues()
{
    //wait(0.01);
    //uint32_t now = ms_since_boot();
    if(!get_subsystem_status(TEST_CAM))
    {
        handle_cam_capture();
        if(CAM_SHUTDOWN){
            camera_enabled(0);

            CAM_SHUTDOWN = 0;
        }
        if(CLOSE_STREAM)
        {
            camera_disable_stream_mode();
            CLOSE_STREAM = 0;
        }
    }

    feed_the_dog();
    check_reset_timer();

    if(!get_subsystem_status(TEST_RADIO))
    {
        radioControl();
    }

    morse_thread();

    if(!get_subsystem_status(TEST_CAM))
    {
        CAM_read_thread();
    }

    
    led_thread();
    payload_handler();
    // Dispatch event queues for 50 ms
    //hkp_queue.dispatch(50);

    //check_tx();
    #ifdef HARD_FLIGHT_MODE
    FLIGHT_MODE_ON = 1;
    #endif
    if(FLIGHT_MODE_ON)
    {
        int now = ms_since_boot();
        if(now - time_of_last_meas >= MEAS_INTERVAL)
        {
            time_of_last_meas = now;
            log_meas();
        }
        if(now - time_of_last_beacon >= BEACON_INTERVAL)
        {
            time_of_last_beacon = now;
            send_beacon();
        }
        if(now - time_of_last_image >= (INTENSIVE_FLIGHT_MODE_ON ? INTENSIVE_IMAGE_INTERVAL : DEFAULT_IMAGE_INTERVAL))
        {
            time_of_last_image = now;
            take_image();
        }
    }
}

float last_pressure = 0;
float last_temperature = 0;
float last_latitude = 0;
float last_longitude = 0;
float last_altitude = 0;
int last_battery = 0;

void log_meas()
{
    char str[256];
    memset(str, 0, sizeof(str));
    // time
    int now = ms_since_boot();

    //bmp
    float pressure = env_get_pressure();
    float temperature = env_get_temp();

    printf("Temp: %f, pres: %f\n\r", temperature, pressure);

    //imu

    /*imu.readAccel();
    imu.readGyro();
    imu.readMag();*/
    float floatBuf[9];
    
    imu_read_mag(floatBuf);
    imu_read_gyro(floatBuf+3);
    imu_read_acc(floatBuf+6);

    float accelX = floatBuf[6];
    float accelY = floatBuf[7];
    float accelZ = floatBuf[8];
    float magX = floatBuf[0];
    float magY = floatBuf[1];
    float magZ = floatBuf[2];
    float gyroX = floatBuf[3];
    float gyroY = floatBuf[4];
    float gyroZ = floatBuf[5];

    //eps

    int eps0 = mcp.read(0);
    int eps1 = mcp.read(1);
    int eps2 = mcp.read(2);
    int eps3 = mcp.read(3);
    int eps4 = mcp.read(4);
    int eps5 = mcp.read(5);
    int eps6 = mcp.read(6);
    int eps7 = mcp.read(7);

    //gps

    uint8_t hour = myGPS.hour;
    uint8_t minute = myGPS.minute;
    uint8_t seconds = myGPS.seconds;

    float latitude = myGPS.latitude;
    float longitude = myGPS.longitude;
    float altitude = myGPS.altitude;
    float speed = myGPS.speed;

    uint8_t fix = myGPS.fix;

    last_pressure = pressure;
    last_temperature = temperature;
    last_latitude = latitude;
    last_longitude = longitude;
    last_altitude = altitude;
    last_battery = eps6;

    //save to sd card
    sprintf(str,"%d,%5.2f,%5.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%4.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%4.5f,%4.5f,%4.2f,%4.2f,%d\r\n",
    now,pressure,temperature,accelX,accelY,accelZ,magX,magY,magZ,gyroX,gyroY,gyroZ,eps0,eps1,eps2,eps3,eps4,eps5,eps6,eps7,hour,minute,seconds,latitude,longitude,altitude,speed,fix
    );

    log_data_to_file(str);
}


int log_data_to_file(char *str)
{
    int err = NO_ERR;
    const char *filepath = LOG_FILE_PATH;
    FILE *fp = fopen(filepath, "a+");
    if(fp == NULL)
    {
        _error("Can't open file for reading: %s\n\r", filepath);
        
        set_subsystem_status(TEST_SD, 1);
        return FILE_OPEN_FAILED;
    }
    fputs(str, fp);
    fclose(fp);

    return 0;
}

void send_beacon()
{
    char beacon[6*4];
    memset(beacon, 0, sizeof(beacon));

    memcpy(beacon+0*4, &last_pressure, 4);
    memcpy(beacon+1*4, &last_temperature, 4);
    memcpy(beacon+2*4, &last_latitude, 4);
    memcpy(beacon+3*4, &last_longitude, 4);
    memcpy(beacon+4*4, &last_altitude, 4);
    memcpy(beacon+5*4, &last_battery, 4);
    
    sendPacket(14, 2, beacon, sizeof((beacon)));
}

void take_image()
{
    TAKE_PIC(NULL, NULL);
}


void hkp_log()
{
    // Take measurements with the modules
    // sshhh

    /*
    // BMP
    bmp_get_temperature();
    bmp_get_pressure();
    
    // GPS
    GPS_read_all();

    // IMU
    imu_read(IMU_ALL);

    // RADIO
    radio_get_temp();
    radio_get_RSSI();

    // ADC
    adc_measure(ADC_ALL);
    */

    // log them to SD card
}

void update_kalman()
{
    // Update kalman filter
    _debug("Kalman update\n\r");
}

void print_seconds_since_boot()
{
    _debug("This program has run %d ms.\n\r", ms_since_boot());
}

void readasasdsd()
{
    read_radio_data();
}

void setup_queues()
{
    //hkp_queue.call_every(1000, readasasdsd);
    //hkp_queue.call_every(1000, update_kalman); // maybe in its own thread tho
    //hkp_queue.call_every(10000, print_seconds_since_boot);

    //tx_queue.call_in(20000, _exit);
    //tx_queue.call_every(15000, broadcast_beacon);
    //tx_queue.chain(&hkp_queue); // Allow concurrent execution
}

// Load settings from SD card. In case non found, fallback to defaults.
void load_settings()
{
    // Create user directory if it doesn't exist
    create_user_directory();
    
    feed_the_dog();
    read_node_id_from_file();

    // Get latest image index if it exists
    int imageCount = get_image_index();
    sprintf(current_image_path, "/fs/mydir/image%d.jpg", imageCount);
    sprintf(current_image_name_only, "image%d.jpg", imageCount);

    int _flight_mode = read_flight_mode_from_file();
    if(_flight_mode)
    {
        int intensive_flight_mode = read_intensive_flight_mode_from_file();
        if(intensive_flight_mode>0)
        {
            write_intensive_flight_mode_to_file(--intensive_flight_mode);
        }
    }

}

