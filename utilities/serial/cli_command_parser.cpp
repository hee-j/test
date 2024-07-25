#include "cli_command_parser.h"

#define WHITESPACE "\n\t\r "

//WARNING: NOT YET IMPLEMENTED.

int parse_cli_command(char* cmd_buf, char *target, char *command, char* _params)
{
    // OBC
    if(  strstr(      cmd_buf,     "reset")         != NULL
      || strstr(      cmd_buf,     "reboot")        != NULL
      || strstr(      cmd_buf,     "restart")       != NULL
    )
    {
        //_debug("Parse Reset OBC\n\r");

        *target     = 1;
        *command    = 1;
        return mOBC_RESET;
    }
    else if(strstr( cmd_buf,     "ping")        != NULL)
    {
        //_debug("Parse Ping OBC\n\r");

        *target     = 1;
        *command    = 5;
        return mOBC_PING;
    }
    else if(strstr( cmd_buf,     "umount")      != NULL)
    {
        *target     = 1;
        *command    = 6;
        return mOBC_UMOUNT;
    }
    else if(strstr( cmd_buf,     "mount")      != NULL && strstr( cmd_buf,     "is_mounted") == NULL )
    {
        *target     = 1;
        *command    = 7;
        return mOBC_MOUNT;
    }
    else if(strstr( cmd_buf,     "is_mounted")      != NULL)
    {
        *target     = 1;
        *command    = 8;
        return mOBC_IS_MOUNTED;
    }
    // CAM
    else if(strstr( cmd_buf,     "list")        != NULL)
    {
        ////_debug  ("Parse Cam take picture\n\r");

        *target     = 2;
        *command    = 1;

        char* token = strtok(strstr(cmd_buf, "list"), WHITESPACE);
        if ( token )
        {
            token =  strtok(NULL, WHITESPACE);
            if( token )
            {
                sprintf(_params, "%s", token);
            } else {
                sprintf(_params, "1");
            }
        } else {
            sprintf(_params, "1");
        }
        return mCAM_LIST_N;
    }
    else if(strstr( cmd_buf,     "num_blocks")        != NULL)
    {
        ////_debug  ("Parse Cam take picture\n\r");

        *target     = 2;
        *command    = 2;

        char* token = strtok(strstr(cmd_buf, "num_blocks"), WHITESPACE);
        if ( token )
        {
            token =  strtok(NULL, WHITESPACE);
            if( token )
            {
                // Img index
                sprintf(_params, "%s", token);
                return mCAM_LIST_N;
            }
        }
        
        _warning("Usage: num_blocks <img_index>\n\r");
        return INVALID_PARAMETERS;
        
    }
    else if(strstr( cmd_buf,     "get_blocks")        != NULL)
    {
        *target     = 2;
        *command    = 3;

        char* token = &(strstr(cmd_buf, "get_blocks"))[11];
        if ( token && strlen(token) > 3)
        { // TODO: more checks... it doesn't check for number of args
            memcpy(_params, token, strlen(token));
            return mCAM_GET_BLOCKS;
        }
        
        
        _warning("Usage: get_blocks <img_index> <block_index>\n\r");
        return INVALID_PARAMETERS;
        
    }
    else if(strstr( cmd_buf,     "take_pic")        != NULL)
    {
        ////_debug  ("Parse Cam take picture\n\r");

        *target     = 2;
        *command    = 4;

        return mCAM_TAKE_PIC;
    }
    /*else if(strstr( cmd_buf,     "heap")        != NULL)
    {
        _debug ("Print heap size\n\r");

        print_memory_info();

        return 6000;
    }*/
    else if(strstr( cmd_buf,     "get_latest")        != NULL)
    {
        ////_debug  ("Parse Cam latest\n\r");

        *target     = 2;
        *command    = 7;

        return mCAM_LATEST;
    }
    // GPS
    else if(strstr( cmd_buf,     "gps_get")     != NULL)
    {
        ////_debug  ("Parse GPS get\n\r");
        *target     = 3;
        *command    = 6;
        
        //char* token = strtok(strstr(cmd_buf, "gps_get"), WHITESPACE);
        //sprintf(_params, "%s", token);
        //////_debug("Params: %s\n\r", token);
        
        // default to 'all'
        return mGPS_ALL;
    }
    // BMP
    else if(strstr( cmd_buf,     "bmp_get")      != NULL)
    {
        ////_debug  ("Parse BMP get\n\r");

        *target     = 4;
        *command    = 3;
        
        char* token = strtok(strstr(cmd_buf, "bmp_get"), WHITESPACE);
        sprintf(_params, "%s", token);
        ////_debug((char*)"Params: %s\n\r", token);
        
        // default to 'all'
        return mBMP_ALL;
    }
    // IMU
    else if(strstr( cmd_buf,     "imu_get")      != NULL)
    {
        ////_debug  ("Parse IMU get\n\r");

        *target     = 5;
        *command    = 14;

        char* token = strtok(strstr(cmd_buf, "imu_get"), WHITESPACE);
        sprintf(_params, "%s", token);

        ////_debug("Params: %s\n\r", token);
        
        // Default to 'all'
        return mIMU_ALL;
    }
    else if(strstr( cmd_buf,     "imu_calib")      != NULL)
    {
        //_debug  ("Parse IMU get\n\r");

        *target     = 5;
        *command    = 13;
        
        return mIMU_CALIB;
    }
    // RADIO
    else if(strstr( cmd_buf,     "rf_get")      != NULL)
    {
        //_debug  ("Parse RADIO get\n\r");

        *target     = 6;
        *command    = 3;

        char* token = strtok(strstr(cmd_buf, "rf_get"), WHITESPACE);
        sprintf(_params, "%s", token);

        //_debug("Params: %s\n\r", token);

        if (strstr(token, "temp") != NULL)
        {
            return mRF_TEMP;
        }
        if (strstr(token, "rssi") != NULL)
        {
            return mRF_RSSI;
        }
        
        // Default to 'all'
        return mRF_ALL;
    }
    // BUZZER
    else if(strstr( cmd_buf,     "beep")      != NULL)
    {
        //_debug  ("Parse BUZZER\n\r");

        *target     = 7;
        *command    = 1;

        char* token = strtok(strstr(cmd_buf, "beep"), WHITESPACE);
        if (token)
        {
            token = strtok(NULL, WHITESPACE);
            if(token && str2int(token) <= 20 && str2int(token) > 1 )
            {
                sprintf(_params, "%s", token);
            } else
            {
                sprintf(_params, "1"); // Default to 1, no usage warnings
            }
        } else {
            sprintf(_params, "1"); // Default to 1, no usage warnings
        }

        //_debug("Params: %s\n\r", token);

        return mBUZZER_BEEP;
    }
    else if(strstr( cmd_buf,     "morse")      != NULL)
    {
        //_debug  ("Parse MORSE\n\r");

        *target     = 7;
        *command    = 3;

        char* token = strstr( cmd_buf,     "morse ");
        token = &(token[6]);
        if (token && strlen(token) > 1)
        {
            sprintf(_params, "%s", token);
        }
        else
        {
            _warning("Usage: morse <string>\n\r");
            return INVALID_PARAMETERS;
        }
        
        //_debug("Params: %s\n\r", token);

        return mBUZZER_MORSE;
    }
    // EPS
    else if(strstr( cmd_buf,     "eps_get")      != NULL)
    {
        //_debug  ("Parse EPS get\n\r");

        *target     = 8;
        *command    = 4;

        char* token = strtok(strstr(cmd_buf, "eps_get"), WHITESPACE);
        if (token)
        {
            token = strtok(NULL, WHITESPACE);
            if(token)
            {
                sprintf(_params, "%s", token);
                //_debug("Params: %s\n\r", token);
                if (strstr(token, "batt_v")      != NULL )
                {
                    *command = 1;
                    return mEPS_BATT_V;
                }
                else if( strstr(token, "sp_v") != NULL)
                {
                    *command = 2;
                    return mEPS_SP_V;
                }
                else if( strstr(token, "sp_c") != NULL)
                {
                    *command = 3;
                    return mEPS_SP_C;
                }
            }
        }
        // default to all
        return mEPS_ALL;
    }
    // LED
    else if(strstr( cmd_buf,     "led_on")        != NULL)
    {
        ////_debug  ("Parse Cam take picture\n\r");

        *target     = 9;
        *command    = 4;

        char* token = strtok(strstr(cmd_buf, "led_on"), WHITESPACE);
        if ( token && strlen(token) > 1)
        {
            if(token)
            {
                int i = 0;
                while(token != NULL)
                {
                    token = strtok(NULL, WHITESPACE);
                    if( token != NULL)
                    {
                        memcpy(_params+i, token, strlen(token));
                        i += strlen(token);
                        memcpy(_params+i, " ", strlen(" "));
                        i += 1;
                    }
                }
            }
            else {
                sprintf(_params, "%d", 0);
            }
            return mLED_LIT;
        }
        
        _warning("Usage: num_blocks <img_index>\n\r");
        return INVALID_PARAMETERS;
        
    }
    else if(strstr( cmd_buf,     "led_off")        != NULL)
    {
        *target     = 9;
        *command    = 1;

        char* token = strtok(strstr(cmd_buf, "led_off"), WHITESPACE);
        if ( token && strlen(token) > 1)
        {
            token =  strtok(NULL, WHITESPACE);
            if( token && strlen(token) > 0)
            {
                sprintf(_params, "%s", token);
            }
        }

        return mLED_STOP;
    }
    else if(strstr( cmd_buf,     "led_start")      != NULL)
    {
        //_debug  ("Parse LED blink\n\r");

        *target     = 9;
        *command    = 2;

        char* token = strtok(strstr(cmd_buf, "led"), WHITESPACE);
        sprintf(_params, "%s", token);

        //_debug("Params: %s\n\r", token);

        return mLED_START;
    }
    // GENERAL
    else if(strstr( cmd_buf,     "whoami")      != NULL)
    {
        *target     = 10;
        *command    = 1;
        return mGENERAL_WHOAMI;
    }
    // GS
    else if(strstr( cmd_buf,     "get_node_id")      != NULL)
    {
        //_debug  ("Parse GS NODE ID\n\r");

        *target     = 11;
        *command    = 3;

        return mGS_GET_NODE_ID;
    }
    else if(strstr( cmd_buf,     "set_node_id")        != NULL)
    {

        *target     = 11;
        *command    = 2;

        char* token = strtok(strstr(cmd_buf, "set_node_id"), WHITESPACE);
        if ( token && strlen(token) > 1)
        {
            token =  strtok(NULL, WHITESPACE);
            if( token && strlen(token) > 0)
            {
                sprintf(_params, "%s", token);

                return mGS_SET_NODE_ID;
            }
        }

        return INVALID_PARAMETERS;
    }
    else if(strstr( cmd_buf,     "get_node_id")        != NULL)
    {

        *target     = 11;
        *command    = 3;

        return mGS_GET_NODE_ID;
    }

    // CLI SPECIFIC COMMANDS.
    else if(strstr( cmd_buf,     "radio_tx")      != NULL)
    {
        char* token = strstr( cmd_buf,     "radio_tx ");
        token = &(token[9]);
        if (token && strlen(token) > 1)
        {
            sprintf(_params, "%s", token);
        }
        else
        {
            _warning("Usage: radio_tx <string>\n\r");
            return INVALID_PARAMETERS;
        }

        return mRADIO_TX;
    }
    else if(strstr( cmd_buf,     "help")      != NULL)
    {
        return mHELP;
    }

    return INVALID_COMMAND;
}