#include "external_payload.h"
#include "hw_config.h"
#include "legacy_comm.h"
#include "serial.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "../../obc/filesystem.h"

UnbufferedSerial payload(EXT_TX, EXT_RX);


char        payload_buf[256];
int         payload_buf_counter = 0;

uint8_t         PAYLOAD_CALLBACK_FLAG = 0;


void send_data(char *data, uint8_t len);
void initialize_rx_buffer();

char       payload_recording_filename[64];
#define    PAYLOAD_RECORDING_FOLDER "/fs/mydir/"
uint8_t    PAYLOAD_RECORDING_ACTIVE = 0;
uint8_t    PAYLOAD_PASSTHROUGH_ACTIVE = 0; 
uint32_t   recorded_payload_data_size = 0; 


void payload_callback()
{
    Timer timeout;
    timeout.reset();
    timeout.start();
    while (timeout.elapsed_time().count() < 20000) { // give the data up to 0.02 seconds (20000us) to arrive.
        if (payload.readable()) {
            char input[1];
            payload.read(input, 1);
            payload_buf[payload_buf_counter] = input[0];
            payload_buf_counter++;
        }
     }
    
    PAYLOAD_CALLBACK_FLAG = 1;
}


// Non ISR context function to handle non-time critical stuff
void payload_handler()
{
    if(PAYLOAD_CALLBACK_FLAG)
    {
        PAYLOAD_CALLBACK_FLAG = 0;
        if(PAYLOAD_PASSTHROUGH_ACTIVE)
        {
            sendPacket(17, 2, payload_buf, payload_buf_counter);
        }
        if(PAYLOAD_RECORDING_ACTIVE)
        {
            FILE *f;
            f = fopen(payload_recording_filename, "ab");
            fwrite(payload_buf, sizeof(char), payload_buf_counter, f);
            fclose(f);
        }
        
        initialize_rx_buffer();
    }
}

void initialize_rx_buffer()
{
    memset(payload_buf, 0, 256);
    payload_buf_counter = 0;
}

void init_external_payload()
{
    payload.baud(115200);
    payload.attach(payload_callback, SerialBase::RxIrq);
    initialize_rx_buffer();
}

void send_data(char *data, uint8_t len)
{
    payload.write(data, len);
}

void SEND_TO_EXTERNAL_PAYLOAD(char *data, uint8_t len)
{
    send_data(data, len);
}

void SET_EXTERNAL_PAYLOAD_PASSTHROUGH(char *_params)
{
    if(strlen(_params) == 0) return;
    if(_params[0] == '0' || not strncmp(_params, "off", 3))
    {
        PAYLOAD_PASSTHROUGH_ACTIVE = 0;
    }else{
        PAYLOAD_PASSTHROUGH_ACTIVE = 1;
    }
}

void ENABLE_EXTERNAL_PAYLOAD_RECORDING(char *_params)
{
    if(strlen(_params) == 0)
    {
        _message("No filename");
        return;
    }
    strcpy(payload_recording_filename, PAYLOAD_RECORDING_FOLDER);
    strcat(payload_recording_filename, _params);

    printf("filename: %s\r\n", payload_recording_filename);

    PAYLOAD_RECORDING_ACTIVE = 1;
}

void DISABLE_EXTERNAL_PAYLOAD_RECORDING(char *_params)
{
    PAYLOAD_RECORDING_ACTIVE = 0;
}

void EXTERNAL_PAYLOAD_RECORDING_SIZE(char *_params)
{
    if(strlen(payload_recording_filename)==0)
    {
        _message("Filename not set");
    }
    uint32_t filesize = readFilesize(payload_recording_filename);
    char msg[64];
    sprintf(msg, "%d", filesize);
    sendPacket(17,7,msg, strlen(msg));
}


void EXTERNAL_PAYLOAD_LATEST_BYTES(char *_params)
{
    if(strlen(payload_recording_filename)==0)
    {
        _message("Filename not set");
    }
    uint32_t filesize = readFilesize(payload_recording_filename);
    uint8_t byterequest = min(filesize, 50);
    
    ifstream is(payload_recording_filename);
    if(is)
    {
        char buf[64];
        is.seekg(-byterequest, is.end);
        is.read(buf, byterequest);
        is.close();
        sendPacket(17,3,buf,byterequest);
    }
}
