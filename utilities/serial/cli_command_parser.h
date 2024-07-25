#ifndef __CLI_COMMAND_PARSER_H_
#define __CLI_COMMAND_PARSER_H_

// WARNING: NOT YET IMPLEMENTED.

#include <string.h>

#include "logging.h"
#include "errors.h"
#include "m_util.h"

enum {
    mOBC_RESET       = 11,
    mOBC_ANSWER_MODE = 12,
    mOBC_LOG_MARK    = 13,
    mOBC_DEBUG_ON    = 14,
    mOBC_PING        = 15,
    mOBC_MOUNT       = 16,
    mOBC_UMOUNT      = 17,
    mOBC_IS_MOUNTED  = 18,

    mCAM_LIST_N      = 21,
    mCAM_NUM_BLOCKS  = 22,
    mCAM_GET_BLOCKS  = 23,
    mCAM_TAKE_PIC    = 24,
    mCAM_RESET       = 25,
    mCAM_PING        = 26,
    mCAM_LATEST      = 27,

    mGPS_TIME        = 31,
    mGPS_LOCATION    = 32,
    mGPS_VELOCITY    = 33,
    mGPS_ALTITUDE    = 34,
    mGPS_FIX         = 35,
    mGPS_ALL         = 36,

    mBMP_TEMP        = 41,
    mBMP_PRESS       = 42,
    mBMP_ALL         = 43,

    mIMU_MAG_ALL     = 51,
    mIMU_CALIB       = 513,
    mIMU_ALL         = 514,

    mRF_TEMP         = 61,
    mRF_RSSI         = 62,
    mRF_ALL          = 63,

    mBUZZER_BEEP     = 71,
    mBUZZER_STOP     = 72,
    mBUZZER_MORSE    = 73,

    mEPS_BATT_V      = 81,
    mEPS_SP_V        = 82,
    mEPS_SP_C        = 83,
    mEPS_ALL         = 84,

    mLED_STOP        = 91,
    mLED_START       = 92,
    mLED_BLINK       = 93,
    mLED_LIT         = 94,

    mGENERAL_WHOAMI  = 101,
    mGENERAL_RESET   = 102,
    mGENERAL_DEV_TYPE= 103,
    mGENERAL_TEST_TX = 104,

    mGS_RST          = 111,
    mGS_SET_NODE_ID  = 112,
    mGS_GET_NODE_ID  = 113,
    mGS_RSSI         = 114,

    // CLI SPECIFIC!
    mHELP            = 200,
    mRADIO_TX        = 400

};

int parse_cli_command(char* cmd, char *target, char *command, char* _params);


#endif