#include "cli.h"

// WARNING: NOT YET IMPLEMENTED.

/*
// Interfaces

// Flags
int         cmd_cb       = 0;

// Global Settings
extern uint8_t  DEBUGGING_ON;

// Radio buffer
extern char     radio_data[64];
extern uint8_t  radio_data_length;

// Local settings
int PRINT_NEWLINE      = 1;

// Global vars for Serial Callback
char        cmd_buf[256];
int         cmd_buf_counter = 0;


*/
/*void print_help_str(void)
{
    _pc.printf("%s\n\r", HELP_STR);
}
*/
/*
void setup_cli()
{
    _pc.printf("Setting up CLI.\n\r");
    memset(cmd_buf, 0, sizeof(cmd_buf));
    //_pc.baud(115200);
    //ISRthread.start(callback(ISR_thread)); First, let's try static approach.
    _pc.attach(serial_callback);
}

void serial_callback()
{
    Timer timeout;
    timeout.reset();
    timeout.start();
    while (timeout.read() < (float)0.02) { // give the data up to 0.02 seconds to arrive.
        if (_pc.readable()) {
            cmd_buf[cmd_buf_counter] = _pc.getc();
            cmd_buf_counter++;
        }
     }
    
    cmd_cb = 1;
    //command_callback();

    //_pc.printf("pc callback: %d\r\n", cmd_buf_counter);
}

int command_callback()
{
    int result = -3;
    if(cmd_buf_counter > 6)
    {
      result = parse_cmd(cmd_buf, cmd_buf_counter);
      
      if(result != -2) //Parse successful
      {
        memset(cmd_buf,0,sizeof(cmd_buf));
        cmd_buf_counter = 0;

      }else if(result == -1) //FNV mismatch, remove the first element of the buffer
      {
       while(result == -1)
        {
                for(int i = 0; i<cmd_buf_counter; i++)
            {
              if(i<255)
              {
                cmd_buf[i] = cmd_buf[i+1];  
              }else{
                cmd_buf[i] = '\0';   
               }
            }
            cmd_buf_counter--;
            result = parse_cmd(cmd_buf, cmd_buf_counter);
        }
        
      }
    } // FLUSH
    
    return result;
}

*/