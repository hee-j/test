#include "settings.h"
#include "logging.h"
#include "errors.h"
#include "logging.h"
#include "test.h"

uint8_t         RADIO_LOADED            = 0;
uint8_t         FILESYSTEM_LOADED       = 0;
uint8_t         DEBUGGING_ON            = 0;
uint8_t         FLIGHT_MODE_ON          = 0;
uint8_t         INTENSIVE_FLIGHT_MODE_ON= 0;

char            transmissionMode        = 0b00010011; // LSB->MSB: UART, RADIO, LOG, BT, Debug

uint8_t         NETWORKID               = 101;

struct stat st = {0};

const char*   node_id_file_path = "/fs/mydir/nodeid";
const char*   flight_mode_file_path = "/fs/mydir/flightmode";
const char*   intensive_flight_mode_file_path = "/fs/mydir/intensiveflightmode";


int write_settings_file(const char *filepath, int value);

int create_user_directory(void)
{
    if (stat("/fs/mydir", &st) == -1)
    {
        mkdir("/fs/mydir", 0700);
        _debug("Creating user directory.\n\r");
    }
    _debug("User directory exists.\n\r");

    return 0;
}

int read_settings_file(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if(fp == NULL)
    {
        write_settings_file(filepath, 0);
        return 0;
    }
    char ch;
    ch = fgetc(fp);

    fclose(fp);
    return ch-'0';
}

int write_settings_file(const char *filepath, int value)
{
    int err = NO_ERR;
    FILE *fp = fopen(filepath, "w+");
    if(fp == NULL)
    {
        _error("Can't open file for reading: %s\n\r", filepath);
        set_subsystem_status(TEST_SD, 1);
        return FILE_OPEN_FAILED;
    }
    fputc(value+'0', fp);
    fclose(fp);

    return 0;
}

int read_node_id_from_file()
{
    NETWORKID = read_settings_file(node_id_file_path);
    _debug("Node ID settings read: %d\n", NETWORKID);

    if(NETWORKID > 0 && NETWORKID <=255)
    {
        setNetworkID(NETWORKID);
    }

    return NETWORKID;
}

int write_node_id_to_file(int node_id)
{
    _debug("Setting node ID to %d.\n\r", node_id);
    return write_settings_file(node_id_file_path, node_id);
}

int read_flight_mode_from_file()
{
    int _flight_mode = read_settings_file(flight_mode_file_path);
    if(_flight_mode)
    {
        FLIGHT_MODE_ON = 1;
    }

    return _flight_mode;
}

int write_flight_mode_to_file(int status)
{
    return write_settings_file(flight_mode_file_path, status);
}

int read_intensive_flight_mode_from_file()
{
    int _intensive_flight_mode = read_settings_file(intensive_flight_mode_file_path);
    if(_intensive_flight_mode)
    {
        INTENSIVE_FLIGHT_MODE_ON = 1;
    }
    return _intensive_flight_mode;
}

int write_intensive_flight_mode_to_file(int status)
{
    return write_settings_file(intensive_flight_mode_file_path, status);
}

int get_image_index(void)
{
  int image_index = 0;
  int temp = 0;
  char digitsonly[10];
  memset(digitsonly, 0, sizeof(digitsonly));
  int k = 0;
  int a = 0;
  
    DIR *d;
    struct dirent *dir;
    d = opendir("/fs/mydir");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {          
           k = 0;
           a = 0;
           char* curr_img = dir->d_name;
           memset(digitsonly, 0, sizeof(digitsonly));
            while (dir->d_name[k] && k < 10)
            {
                if(isDigit(dir->d_name[k]))
                {
                    char c = dir->d_name[k];
                    digitsonly[a] = c;
                    a++;
                }
                k++;
            }
            digitsonly[a] = '\0';
            image_index = max(image_index, str2int(digitsonly));
        }
        closedir(d);
    }

    return image_index+1;
}

void SET_FLIGHT_MODE(char *_params)
{
    _debug("Setting flight mode");
    int mode = 0;
    mode = str2int(_params);
    INTENSIVE_FLIGHT_MODE_ON = 0;
    FLIGHT_MODE_ON = 0;
    
    if(mode)
    {
        FLIGHT_MODE_ON = 1;
    }
    write_flight_mode_to_file(FLIGHT_MODE_ON);

    if(mode < 0)
    {
        mode = 0;
    }

    if(mode > 0)
    {
        INTENSIVE_FLIGHT_MODE_ON = 1;
        write_intensive_flight_mode_to_file(mode);
    }

    char _ans[32];
    
    sprintf(_ans, "set flight mode to %d", mode);
    _debug(_ans);
}