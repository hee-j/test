/***
Tessa Nikander, Samuli Nyman
2019-2022
***/

#include "mbed.h"
#include "payload/radio/RadioController.h"
#include "cmd_interpreter.h"
#include "mbed_mem_trace.h"
#include "obc/wd.h"
#include "utilities/serial/logging.h"
#include "utilities/serial/serial.h"
#include "payload/radio/radio.h"
#include "utilities/test.h"
#include "utilities/errors.h"
#include "payload/buzzer.h"
#include "obc/filesystem.h"
#include "loop.h"
#include "obc/sdhc.h"
#include "eps/mcp.h"
#include "obc/camera.h"
#include "payload/led.h"
#include "payload/gps.h"
#include "payload/imu.h"
#include "settings.h"
#include "payload/env.h"
#include "external_payload.h"

extern  uint8_t     RADIO_NEW_DATA; // RadioController.cpp
extern char            radio_data[64]; // RadioController.cpp
extern uint8_t         radio_data_length; // RadioController.cpp

// Settings
extern  uint8_t     RADIO_LOADED; // settings.cpp
extern  uint8_t     FILESYSTEM_LOADED;

// Flags
bool                done        = false;
extern int          COMMAND_CALLBACK_FLAG;
extern uint8_t      DEBUGGING_ON;

// Interfaces
extern UnbufferedSerial           _pc;

uint8_t SETUP_DONE          = 0;
int receive_count           = 0;

// Common with sat
///extern Timer        transmitTimer;
//extern RFM69         radio;
extern char            radio_data[64];
extern uint8_t         radio_data_length;
extern DigitalOut      cam_en;
//extern DigitalOut      burn;

//*******  Initialization and setup ********/
// i) DESCRIPTION
// Load all modules. Proceed even if some modules fail to load.
//
// ii) MODULES
// Critical modules:
//  - Core and utils
//  - RFM69
//  - File system
// Non-critical modules:
//  - MCP3008
//  - LSM9DS1
//  - OV2640
//  - BMP280
//  - Adafruit Ultimate GPS
//  - Kitsat LED driver
//  - Kitsat Bluetooth

int setup(void)
{
    int err = 0;
    /*********** CRITICAL MODULES ********/
    printf("Starting serial\r\n");
    setup_serial();
    printf("Success in bootloader\r\n");
    // CLI
    //setup_cli();

    
    // Hardware watchdog
    printf("Starting watchdog\r\n");
    wd_init();

    feed_the_dog();


    printf("Starting radio... ");
    // Load radio module
    int8_t radio_status = radio_init();
    set_subsystem_status(TEST_RADIO, radio_status, true);
    radio_status        == NO_ERR ? RADIO_LOADED        = 1 : RADIO_LOADED;

    feed_the_dog();
    // Init filesystem
    printf("Starting filesystem... ");
    int8_t filesystem_status = filesystem_mount();
    set_subsystem_status(TEST_SD, filesystem_status, true);
    filesystem_status  == NO_ERR ? FILESYSTEM_LOADED   = 1 : FILESYSTEM_LOADED;
    
    feed_the_dog();
    test_filesystem();

    return err;
    
}

void print_module_errors()
{
    if(!FILESYSTEM_LOADED)
    {
        _error("Unable to mount filesystem. Please make sure that the SD card is attached and formatted to FAT.\n\r");
    }
    if(!RADIO_LOADED)
    {
        _error("Unable to load radio module. Check your connections and power levels.\n\r");
    }
}

void radio_received() // Filename...
{
    parse_cmd(radio_data, radio_data_length);
    RADIO_NEW_DATA = 0;
    fflush(stdout);
}

void setup_noncritical()
{

    /********** NON-CRITICAL MODULES ****/
    // Don't crash the system even if these modules don't load.

    // Set buzzer off
    printf("Starting buzzer\r\n");
    buzzer_init();

    // Init MCP adc
    printf("Starting ADC... ");
    set_subsystem_status(TEST_ADC, 
    adc_init(), true);

    feed_the_dog();

    //feedTheDog();
    // Init Camera
    printf("Starting camera... ");
    set_subsystem_status(TEST_CAM, 
    camera_setup(), true);
    feed_the_dog();
    // LEDs are using same SPI with the camera
    // and LED Driver max SPI speed is only 10MHz.
    // Remember to change SPI speeds when initiating writes and reads.
    printf("Starting leds\r\n");
    leds_init();

    // Init GPS
    printf("Starting gps\r\n");
    GPS_init();
    feed_the_dog();

    // Roblmps
    // Init IMU 9DOF
    printf("Starting imu... ");
    set_subsystem_status(TEST_IMU, 
    imu_init(), true);

    feed_the_dog();

    // Init the pressure sensor
    printf("Starting env... ");
    set_subsystem_status(TEST_BMP, env_init(), true);
    feed_the_dog();

    // Init the external payload uart
   
    printf("Starting external payload interface...");
    init_external_payload();
    printf(" Done\r\n");
    

    // KITSAT CONFIGURATION

    // Settings
    // Creates user directory, if it doesn't exist.
    // Gets number of latest image in disk, if it exists.
    feed_the_dog();
    printf("Loading settings\r\n");
    load_settings(); // loop.cpp
    feed_the_dog();
    print_module_errors();
}
 
int booleanize(int val)
{
    if(val)
    {
        return 1;
    }else{
        return 0;
    }
}

time_t read_rtc(void) {
    return 0;
}

// dev tool to do simple tests for new interfaces
Timeout test_function_timeout;
uint8_t TEST_FUNCTION_FLAG = 0;
#define TEST_FUNCTION_DELAY 1s

void test_function()
{
    
}

void test_function_flag_set()
{
    TEST_FUNCTION_FLAG = 1;
}

int main()
{
    // hack the spec - dirty ass trick to fix the filesystem bug by breaking the RTC
    attach_rtc(&read_rtc, NULL, NULL, NULL);

    start_boot_timer();

    //mbed_mem_trace_set_callback(&mbed_mem_trace_default_callback);

    //DEBUGGING_ON = 1;
    cam_en = 1;
    //burn = 0;
    setup();
    printf("Set up non-critical functions\r\n");
    setup_noncritical();

    env_get_temp(); // hack the bmp

    //print_memory_info();
    printf("check errors\r\n");
    if(system_has_errors()){
        GET_SYSTEM_STATUS();
    }else{
        quick_beep();
    }
    printf("start loop\r\n");

//    test_function_timeout.attach(test_function_flag_set, TEST_FUNCTION_DELAY);
    while(!done)
    {
        manage_queues();
        if( COMMAND_CALLBACK_FLAG )
        {
            command_callback();
            COMMAND_CALLBACK_FLAG = 0;
            fflush(stdout);
        }
/*
        if(TEST_FUNCTION_FLAG)
        {
            TEST_FUNCTION_FLAG = 0;
            test_function_timeout.attach(test_function_flag_set, TEST_FUNCTION_DELAY);
            test_function();
        }
*/
    }

    printf("All done.\r\n");
    cleanup();
    _debug("Main loop exited.\n\rAll done.\n\r");
}