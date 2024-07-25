#include "serial.h"
#include "hw_config.h"
#include "cmd_interpreter.h"

// Interfaces
UnbufferedSerial   _pc(PC_TX, PC_RX);

char        cmd_buf[256];
int         cmd_buf_counter = 0;

int         COMMAND_CALLBACK_FLAG;

void        serial_callback();

void setup_serial()
{
    _pc.baud(115200);
    _pc.attach(serial_callback);
}

void serial_callback()
{
    
    Timer timeout;
    timeout.reset();
    timeout.start();
    while (timeout.elapsed_time().count() < 20000) { // give the data up to 0.02 seconds (20000us) to arrive.
        if (_pc.readable()) {
            char input[1];
            _pc.read(input, 1);
            cmd_buf[cmd_buf_counter] = input[0];
            cmd_buf_counter++;
        }
     }
    
    COMMAND_CALLBACK_FLAG = 1;
    //command_callback();

    //_pc.printf("pc callback: %d\r\n", cmd_buf_counter);
}

void pc_printf(const char *fmt, ...)
{
    va_list args;
    const char *p;
    va_start(args, fmt);
    char message[256];
    
    for(p = fmt; *p; p++)
    {
        sprintf(message, fmt, p);
    }

    _pc.write(message, strlen(message));

    va_end(args);
}

void pc_putc(char chr)
{
    _pc.write(&chr, 1);
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
