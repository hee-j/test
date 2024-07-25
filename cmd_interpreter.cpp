#include "cmd_interpreter.h"

#include "utilities/m_util.h"
#include "eps/mcp.h"
#include "utilities/submodule.h"
#include "utilities/flags.h"
#include "utilities/serial/logging.h"
#include "utilities/errors.h"
#include "utilities/version.h"
#include "utilities/test.h"
#include "obc/filesystem.h"
#include "utilities/serial/external_payload.h"
#include "payload/radio/legacy_comm.h"

// Submodules in case executed in this context.
#include "payload/env.h"
#include "payload/buzzer.h"
#include "payload/led.h"
#include "payload/radio/radio.h"
#include "payload/imu.h"
#include "payload/gps.h"
#include "obc/camera.h"
#include "obc/obc.h"

extern uint8_t      NETWORKID;

// Refer to:
// https://docs.google.com/spreadsheets/d/10JQrkpLVDIyB03xetAFTzuSk0sgWTfwx5vMFuJEL8vY/edit#gid=1446024382
// For list of available commands.

// Legacy parse cmd
int parse_cmd(char *bts, int length)
{
    if(length<2) return PACKET_NOT_COMPLETE;

    //_debug("Interpreting %s...\n\r", bts);
    
    int target_id = bts[0];
    int command_id = bts[1];
    int data_len = bts[2];
    
    if(length<data_len+7)
    {
        _debug("PACKET NOT COMPLETE!!!\n\r");
        return PACKET_NOT_COMPLETE;
    }
    
    char parameters[data_len+1];
    for(int i=0; i<data_len; i++)
    {
        parameters[i]=bts[i+3];
    }
    
    parameters[data_len] = '\0';
    
    uint32_t _fnv = (bts[data_len+6] << 24) | (bts[data_len+5] << 16) | (bts[data_len+4] << 8) | (bts[data_len+3]);
    uint32_t local_fnv = ufnv(bts, data_len+3);
    
    if(_fnv != local_fnv) {
        _debug("FNV MISMATCH!!!\n\r");
        return FNV_MISMATCH;   
    }

    // Packet done
    //print_memory_info();
    
    
    interpret(target_id, command_id, parameters, data_len); //Actual interpret
    //sendPacket(target_id, command_id, parameters, data_len); // echo command for _debug(ging
    
    return 0;
}

void interpret(uint32_t target, uint32_t cmd, char *_params, int data_len)
{
    switch(target)
    {
        case OBC:
            //_debug("INTERPRET OBC\n\r");
            cmd_OBC(cmd, _params);
            return;
        case CAM:
            //_debug("INTERPRET CAM\n\r");
            cmd_CAM(cmd, _params);
            return;
        case GPS:
            //_debug("INTERPRET GPS\n\r");
            cmd_GPS(cmd, _params);
            return;
        case _BMP:
            //_debug("INTERPRET BMP\n\r");
            cmd_BMP(cmd, _params);
            return;
        case IMU:
            //_debug("INTERPRET IMU\n\r");
            cmd_IMU(cmd, _params);
            return;
        case RF:
            //_debug("INTERPRET RADIO\n\r");
            cmd_RF(cmd, _params);
            return;
        case BUZZER:
            cmd_BUZZER(cmd, _params);
            //_debug("INTERPRET BUZZER\n\r");
            return;
        case EPS:
            //_debug("INTERPRET EPS\n\r");
            cmd_EPS(cmd, _params);
            return;
        case LED:
            //_debug("INTERPRET LED\n\r");
            cmd_LED (cmd, _params);
            return;
        case GENERAL:
            //_debug("INTERPRET GENERAL\n\r");
            cmd_General(cmd, _params);
            return;
        case GS:
            //_debug("INTERPRET GS\n\r");
            cmd_GS(cmd, _params);
            return;
        case BT:
            //_debug("INTERPRET BT\n\r");
            cmd_BT(cmd, _params);
            return;
        case FILETRANSFER:
            cmd_FILETRANSFER(cmd, _params, data_len);
            return;
        case EXTERNAL_PAYLOAD:
            cmd_EXTERNAL_PAYLOAD(cmd, _params, data_len);
            return;
        default:
            _error("Invalid target id: %d\n\r", target);
            return;
    }
}

// FILETRANSFER
void cmd_EXTERNAL_PAYLOAD(uint32_t cmd, char* _params, uint16_t data_len)
{
    switch(cmd)
    {
        case 1:
            SEND_TO_EXTERNAL_PAYLOAD(_params, data_len);
            return;
        case 3:
            EXTERNAL_PAYLOAD_LATEST_BYTES(_params);
            return;
        case 4:
            SET_EXTERNAL_PAYLOAD_PASSTHROUGH(_params);
            return;
        case 5:
            ENABLE_EXTERNAL_PAYLOAD_RECORDING(_params);
            return;
        case 6:
            DISABLE_EXTERNAL_PAYLOAD_RECORDING(_params);
            return;
        case 7:
            EXTERNAL_PAYLOAD_RECORDING_SIZE(_params);
            return;
        default:
            _error("Invalid param %d, EXT\n\r", cmd);
            return;
    }
}

// FILETRANSFER
void cmd_FILETRANSFER(uint32_t cmd, char* _params, int data_len)
{
    switch(cmd)
    {
        case 1:
            INITIATE_UPLINK(_params);
            return;
        case 2:
            UPLINK(_params, data_len);
            return;
        case 3:
            UPLINK_STATUS();
            return;
        case 4:
            UPLINK_FNV();
            return;
        case 5:
            FINALIZE_UPLINK();
            return;
        case 6:
            PRINT_FILE();
            return;
        case 7:
            DELETE_UPLINK_FILE();
            return;
        case 8:
            NEXT_BLOCKS();
            return;
        default:
            _error("Invalid param %d, GS\n\r", cmd);
            return;
    }
}

// BLUETOOTH
void cmd_BT(uint32_t cmd, char* _params)
{
    _warning("BT commands not yet implemented.\n\r");
}

// GS
void cmd_GS(uint32_t cmd, char* _params)
{
    char aa[32];
    sprintf(aa, "%d", NETWORKID);
    char nodeid[2];
    
    switch(cmd)
    {
        case 1: // local reset
            _error("GS reset not yet implemented\n\r", cmd);
            return;
        case 2:
            _error("GS set node id not yet implemented.");
            return;
        case 3:
            _debug("Node ID: %s\n\r", aa);
            sendPacket(GS, 3, aa, strlen(aa) );
            return;
        case 4:
            _error("GS RSSI not yet implemented\n\r", cmd);
            return;
        default:
            _error("Invalid param %d, GS\n\r", cmd);
            return;
    }
}

// General
void cmd_General(uint32_t cmd, char *_params)
{
    switch (cmd)
    {
        case 1:
            _debug("Whoami\n\r");
            WHOAMI(cmd);
            return;
        case 2:
            //_debug("Reset\n\r");
            _RESET();
            return;
        case 3:
            _debug("Ping\n\r");
            _message("1", GENERAL, 3);
            return;
        case 5:
            //burn10s();
            _message("BURNING\r\n", GENERAL, 5);
            return;
        case 6:
            UPTIME();
            return;
        case 9:
            GET_NODE_ID();
            return;
        case 10:
            if( write_node_id_to_file(str2int(_params)) )
                _message("ok", GS, 2);
            else
                _message("fail", GS, 2);
            return;
        default:
            // Invalid cmd
            _error("Invalid param %d, General\n\r", cmd);
            return;
    }
}

// obc.cpp
void cmd_OBC(uint32_t cmd, char *_params)
{
    switch(cmd) {
        case 1:
            //_debug("Reset\n\r");
            _RESET();
            return;
        case 2:
            //_debug("Answer mode\n\r");
            ANSWER_MODE(_params);
            return;
        case 3:
            //_debug("Mark to log\n\r");
            MARK_TO_LOG(_params);
            return;
        case 4:
            //_debug("Set debug messages on/off\n\r");
            DEBUG_MESSAGES_ON(_params);
            return;
        case 5:
             //_debug("Ping\n\r");
            PING();
            return;
        case 6:
            UNMOUNT();
            return;
        case 7:
            MOUNT();
            return;
        case 8:
            IS_MOUNTED();
            return;
        case 9:
            GET_NETWORK_ID();
            return;
        case 10:
            SET_NETWORK_ID(_params);
            return;
        case 11:
            GET_VERSION();
        default:
            _error("Invalid param %d, OBC\n\r", cmd);
            return;
    }
}


//camera.cpp
void cmd_CAM(uint32_t cmd, char *_params)
{
    switch (cmd)
    {
        case 1:
            //_debug("Cam list n\n\r");
            LIST_N(cmd, _params);
            return;
        case 2:
            //_debug("Cam num blocks\n\r");
            NUM_BLOCKS(cmd, _params);
            return;
        case 3:
            //_debug("Cam get blocks\n\r");
            BLOCKS(cmd, _params);
            return;
        case 4:
            //_debug("Cam take pic\n\r");
            TAKE_PIC(cmd, _params);
            return;
        case 5:
            //_debug("Cam reset\n\r");
            CAM_RESET(cmd, _params);
            return;
        case 6:
            //_debug("Cam ping\n\r");
            CAM_PING(cmd, _params);
            return;
        case 7:
            //_debug("Cam latest\n\r");
            CAM_LATEST(cmd, _params);
            return;
        case 12:
            STREAM_PIC(cmd, _params);
            return;
        default:
            _error("Invalid param %d, CAM\n\r", cmd);
            return;
    }

}

// gps.cpp
void cmd_GPS(uint32_t cmd, char *_params)
{
    switch(cmd)
    {
        case 1:
            //_debug("GPS time\n\r");
            GPS_TIME(_params);
            return;
        case 2:
            //_debug("GPS location\n\r");
            GPS_LOCATION(_params);
            return;
        case 3:
            //_debug("GPS velocity\n\r");
            GPS_VELOCITY(_params);
            return;
        case 4:
            //_debug("GPS altitude\n\r");
            GPS_ALTITUDE(_params);
            return;
        case 5:
            //_debug("GPS fix\n\r");
            GPS_FIX(_params);
            return;
        case 6:
            //_debug("GPS all\n\r");
            GPS_ALL(_params);
            return;
        default:
            _error("Invalid param %d, GPS\n\r", cmd);
            break;
    }
}

// bmp280.cpp
void cmd_BMP(uint32_t cmd, char *_params)
{
    switch(cmd)
    {
        case 1:
            //_debug("BMP temp\n\r");
            ENV_TEMP();
            return;
        case 2:
            //_debug("BMP press\n\r");
            ENV_PRESS();
            return;
        case 3:
            //_debug("BMP all\n\r");
            ENV_ALL();
            return;
        default:
            _error("Invalid param %d, BMP\n\r", cmd);
            return;
    }
}

// imu.cpp
void cmd_IMU(uint32_t cmd, char *_params)
{
    switch(cmd)
    {
        case mag_all:
            _IMU_MAG();
            return;
        case gyr_all:
            _IMU_GYR();
            return;  
        case acc_all:
            _IMU_ACC();
            return;  
        case calib:
            //_debug("IMU CALIB\n\r");
            _IMU_CALIB();
            return;
        case imu_all:
            //_debug("IMU ALL\n\r");
            _IMU_ALL();
            return;
        default:
            _error("Invalid param %d, IMU\n\r", cmd);
            return;  
    }

}

// radio.cpp
void cmd_RF(uint32_t cmd, char *_params)
{
    switch(cmd) {
        case 1:
            //_debug("RF temp\n\r");
            TEMP_RF();
            return;
        case 2:
            //_debug("RF RSSI\n\r");
            RSSI_RF();
            return;
        case 3:
            //_debug("RF All\n\r");
            RF_ALL();
            return;
        default:
            _error("Invalid param %d, Radio\n\r", cmd);
            return;
    }

}

// buzzer.cpp
void cmd_BUZZER(uint32_t cmd, char *_params)
{
    switch(cmd) {
        case 1:
            //_debug("beep\n\r");
            BEEP(_params);
            return;
        case 2:
            //_debug("stop beep\n\r");
            STOP_BEEP(_params);
            return;
        case 3:
            //_debug("morse\n\r");
            MORSE(_params);
            return;
        default:
            _error("Invalid param %d, buzzer\n\r", cmd);
            return;
    }

}

// mcp.cpp
void cmd_EPS(uint32_t cmd, char *_params)
{
    switch(cmd)
    {
        case battery_voltage:
            //_debug("Batt V\n\r");
            BATT_V(_params);
            return;
        case 2:
            //_debug("SP V\n\r");
            SP_V(_params);
            return;
        case 3:
            //_debug("SP C\n\r");
            SP_C(_params);
            return;
        case 4:
            //_debug("EPS all\n\r");
            EPS_MEASURE_ALL();
            return;
        case 5:
            GET_CHARGING_STATUS();
            return;
        default:
            _error("Invalid param %d, EPS\n\r", cmd);
            return;
    }

}

// led.cpp
void cmd_LED(uint32_t cmd, char *_params)
{
    switch(cmd)
    {
        case LED_stop:
            //_debug("Led stop\n\r");
            LED_STOP(_params);
            return;
        case LED_start:
            //_debug("Led start\n\r");
            LED_START(_params);
            return;
        case 3:
            //_debug("Led blink\n\r");
            LED_BLINK(_params);
        case 4:
            //_debug("Led lit\n\r");
            LED_LIT(_params);
            return;
        default:
            _error("Invalid param %d, LED\n\r", cmd);
            return;
    }
}