#include "camera.h"
#include "mbed_mem_trace.h"
#include <fstream>
#include "loop.h"
#include "serial.h"

int read_fifo_burst(char *filepath);

// OV2640 interfaces
I2C cami2c(CAM_I2C_SDA, CAM_I2C_SCL);
SPI camspi(CAM_SPI_MOSI, CAM_SPI_MISO, CAM_SPI_SCK);

ArduCAM myCAM(OV2640, CAM_SPI_CS, &camspi, &cami2c);



// Add digitalout because mbed hates everyone
DigitalOut cam_cs(CAM_SPI_CS);
DigitalOut cam_en(CAM_EN);

// img RAM buf
char *imgBuf = NULL;

// flags.cpp
extern uint8_t __CAM_CAPTURE_FLAG;
extern uint8_t CAM_ENABLED;
uint8_t CAPTURE_AND_STREAM = 0;
uint8_t camera_is_on = 0;
uint8_t stream_on = 0;

/* 
// Threads
Thread      ISRCAMthread(osPriorityAboveNormal, 1024*4);
osThreadId  ISRCAMthreadId;
*/
Timeout camera_timeout;
Timeout stream_timeout;

void        CAM_read_thread();

char        mfilename[50];
char        mfilepath[64];
        

extern      uint8_t CAM_ENABLED;
extern      uint8_t CAM_SHUTDOWN;
extern      uint8_t CLOSE_STREAM;
extern      uint8_t FLIGHT_MODE_ON;

// Adapted from:
// https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/examples/mini/ArduCAM_Mini_2MP_OV2640_functions/ArduCAM_Mini_2MP_OV2640_functions.ino

// And also adapted from:
// https://github.com/ArduCAM/Arduino/issues/332
// And also:
// https://os.mbed.com/users/dflet/code/cc3100_Test_websock_Camera_CM3/file/e448e81c416f/camera_app/camera_app.cpp/

int CAM_READ = 0;

int CAM_READ_ENABLED = 0;

#define DEFAULT_RESOLUTION OV2640_1280x1024
#define STREAM_RESOLUTION OV2640_320x240

int camera_save_initiated(char *__filepath)
{
      
      if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
      {
        if(CAPTURE_AND_STREAM == 1)
        {
            CAM_stream_pic();
            myCAM.clear_fifo_flag();
            CAPTURE_AND_STREAM = 0;
         //   myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);

            return 1;

        }else
        {
            _debug("ACK CMD CAM Capture Done.\n\r");
            sprintf(mfilename, "%s", __filepath);
            sprintf(mfilepath, "/fs/mydir/%s", mfilename);

            // don't send the filename in flight mode
            if(!FLIGHT_MODE_ON)
            {
                sendPacket(CAM, 4, mfilename, strlen(mfilename));
            }

            // Start cam thread.
            CAM_READ_ENABLED = 1;
            //osSignalSet(ISRCAMthreadId, 0x01);
            //err = read_fifo_burst();
            //Clear the capture done flag
            myCAM.clear_fifo_flag();

            return 1;
        }
      }
      
      return 0;
}

void CAM_stream_pic()
{
    uint32_t length = 0;
    uint16_t temp = 0, temp_last = 0;
    uint8_t FILE_NOT_STARTED = 1;
    uint16_t DOG_COUNTER = 0;
    length = myCAM.read_fifo_length();

    myCAM.cs_low();
    cam_cs = 0;

    char length_string_buf[8];
    sprintf(length_string_buf, "%d", length);
    sendPacket(CAM, 12, length_string_buf, sizeof(length_string_buf));

    
    pc_printf("stream");

    while(1)
    {
        temp_last = temp;
        temp = myCAM.read_fifo();

        pc_putc(temp);

        if(DOG_COUNTER++>2048)
        {
            feed_the_dog();
            DOG_COUNTER = 0;
        }
        
        if( temp == 0xD9 && temp_last == 0xFF ){  
            break;
        }
    }

    myCAM.flush_fifo();
                        
    myCAM.cs_high();
    cam_cs = 1;


}

void CAM_read_thread() // Filename...
{
    //ISRCAMthreadId = osThreadGetId();
/* 
    while(1)
    {
        osSignalWait(0x01, osWaitForever);
        */
    if(CAM_READ_ENABLED)
    {

        pc_printf("Reading. File name: %s\n\r", mfilename);

        fflush(stdout);
        FILE *fp          = fopen(mfilepath, "wb"); 
        
        if( fp == NULL)
        {
            _error("Could not open file %s for write\n\r", mfilename);
        }
        else
        {
            uint16_t temp = 0, temp_last = 0;
            uint32_t length = 0;
            length = myCAM.read_fifo_length();
            //printf(length, DEC);
            pc_printf("Len: %d\n\r", length);
            if (length >= MAX_FIFO_SIZE) //512 kb
            {
                pc_printf("ACK CMD Over size.\n\r");
                fclose(fp);
            }
            else
            {
                if (length == 0 ) //0 kb
                {
                    pc_printf("ACK CMD Size is 0.\n\r");
                    fclose(fp);
                }
                else
                {
                    myCAM.cs_low();
                    cam_cs = 0;

                    int chunk_index = 0;
                    char chunk_buffer[53];
                    memset(chunk_buffer,0,sizeof(chunk_buffer));

                    imgBuf = (char*) malloc( LARGE_IMG_BUF_SIZE*sizeof(char)+1 );

                    if( imgBuf == NULL )
                    {
                        pc_printf("Not enough memory for imagery functions\n\r");
                        fclose(fp);
                    }
                    else
                    {

                        //myCAM.set_fifo_burst();    
                            int i = 0;
                            int j = 0;  // for count the byte read from fifo,
                                        // unlike 'i', it does not reset to 0
                                        // after a partial image
                            uint8_t only_null = 1;
                            while( 1 )
                            {     
                                /*
                                if(i % 1000 == 0)
                                {
                                printf("i: %05d, j: %05d\n", i, j);   
                                }*/
                                temp_last = temp;    
                            // temp = (uint8_t )myCAM._cam_spi.write(0x00);
                                temp = myCAM.read_fifo();
                                if(only_null && temp == '\0'){
                                    continue;
                                }
                                only_null = 0;
                                imgBuf[i] = temp;
                                i++;
                                j++;

                                 feed_the_dog();
                                
                                // deliberately use a smaller 'i' to create more "image breaking"
                                //if(i == 55000)
                                if(i == 10240)
                                //if(i == LARGE_IMG_BUF_SIZE)
                                {
                                    fwrite(imgBuf, sizeof(char), i, fp);
                                    // somehow a call to "myCAM.read_fifo_length()" solves the
                                    // image breaking problem... reason not clear yet...
                                    pc_printf("After write: %d\n\r", myCAM.read_fifo_length());
                                    //myCAM.read_fifo_length();
                                    
                                    i = 0;
                                }
                                
                                if( temp == 0xD9 && temp_last == 0xFF ){  
                                    pc_printf("EOI marker found:: i: %05d, j: %05d\n\r", i, j); 
                                break;
                                }
                            }
                            
                            // Final image data write
                            fwrite(imgBuf, sizeof(char), i, fp);
                            pc_printf("After final image write,\n\r");
                            //pc_printf("fifo length originally, size now: %5d, %5d\n\r", length, myCAM.read_fifo_length());
                            
                            // there is some remaining bytes after the "FF D9" EOI marker
                            // lets print them out 
                            // I used to write them to the jpeg file as well,
                            // but now I have commented that part out
                            int pdata_size = length - j;
                            pc_printf("The pdata in hex:\n\r");
                            for(int k=0; k < pdata_size; k++)
                            {
                                temp_last = temp;    
                                temp = myCAM.read_fifo();
                                imgBuf[i] = temp;
                                pc_printf("%02X ", temp);
                                if (k%15 == 0) printf("\n\r");
                                i++;
                                j++;
                            }
                            pc_printf("\n");
                            pc_printf("i: %05d, j: %05d\n", i, j);
                            pc_printf("After reading post EOI data: %d\n", myCAM.read_fifo_length());
                            pc_printf("%.*s\n", length - j, imgBuf);
                            // fwrite(imgBuf, sizeof(char), pdata_size, fp);
                            pc_printf("After processing writing pdata,\n");
                            pc_printf("fifo length originally, size now, j: %5d, %5d\n", length, myCAM.read_fifo_length(), j);

                        fclose(fp);

                        if(imgBuf)
                        {
                            pc_printf("Freeing image buf.\n\r");
                            free(imgBuf);
                        } else {
                            pc_printf("NO IMAGE BUF TO FREE!!!\n\r");
                        }
                        
                        myCAM.flush_fifo();
                        
                        myCAM.cs_high();
                        cam_cs = 1;

                        feed_the_dog();

                       // _debug("Save finished.\n\r");

                        load_settings(); // Update newest image name
                    }
                }
            }
        }
        CAM_READ_ENABLED = 0;
    }
}


int read_fifo_burst(char *filename)
{
  printf("File name: %s\n\r", filename);
  //char chunkpath[128];
  //sprintf(chunkpath, "/fs/mydir/chunks/%s.chunks", filename);

  char filepath[64];
  sprintf(filepath, "/fs/mydir/%s", filename);

  fflush(stdout);
  FILE *fp          = fopen(filepath, "wb"); 
  //FILE *chunkfile   = fopen(chunkpath, "wb");
  
  if( fp == NULL)
  { 
      _error("Could not open file %s for write\n\r", filename);
      return FILE_OPEN_FAILED;
  }

  uint16_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  length = myCAM.read_fifo_length();
  //printf(length, DEC);
  printf("Len: %d\n\r", length);
  if (length >= MAX_FIFO_SIZE) //512 kb
  {
    printf("ACK CMD Over size.\n\r");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    printf("ACK CMD Size is 0.\n\r");
    return 0;
  }
  myCAM.cs_low();
  cam_cs = 0;

  int chunk_index = 0;
  char chunk_buffer[53];
  memset(chunk_buffer,0,sizeof(chunk_buffer));

  //myCAM.set_fifo_burst();    
    int i = 0;
    int j = 0;  // for count the byte read from fifo,
                // unlike 'i', it does not reset to 0
                // after a partial image
    while( 1 )
    {     
        
        if(i % 1000 == 0)
        {
          printf("i: %05d, j: %05d\n", i, j);   
        }
        temp_last = temp;    
       // temp = (uint8_t )myCAM._cam_spi.write(0x00);
        temp = myCAM.read_fifo();
        imgBuf[i] = temp;
        i++;
        j++;

        feed_the_dog();
        
        // deliberately use a smaller 'i' to create more "image breaking"
        //if(i == 55000)
        if(i == 10240)
        //if(i == LARGE_IMG_BUF_SIZE)
        {
            fwrite(imgBuf, sizeof(char), i, fp);
            // somehow a call to "myCAM.read_fifo_length()" solves the
            // image breaking problem... reason not clear yet...
            printf("After write: %d\n", myCAM.read_fifo_length());
            //myCAM.read_fifo_length();
            
            i = 0;
        }
        
        if( temp == 0xD9 && temp_last == 0xFF ){  
            printf("EOI marker found:: i: %05d, j: %05d\n", i, j); 
           break;
        }
    }
    
    // Final image data write
    fwrite(imgBuf, sizeof(char), i, fp);
    printf("After final image write,\n");
    printf("fifo length originally, size now: %5d, %5d\n", length, myCAM.read_fifo_length());
    
    // there is some remaining bytes after the "FF D9" EOI marker
    // lets print them out 
    // I used to write them to the jpeg file as well,
    // but now I have commented that part out
    int pdata_size = length - j;
    printf("The pdata in hex:\n");
    for(int k=0; k < pdata_size; k++)
    {
        temp_last = temp;    
        temp = myCAM.read_fifo();
        imgBuf[i] = temp;
        printf("%02X ", temp);
        if (k%15 == 0) printf("\n");
        i++;
        j++;
    }
    printf("\n");
    printf("i: %05d, j: %05d\n", i, j);
    printf("After reading post EOI data: %d\n", myCAM.read_fifo_length());
    printf("%.*s\n", length - j, imgBuf);
    // fwrite(imgBuf, sizeof(char), pdata_size, fp);
    printf("After processing writing pdata,\n");
    printf("fifo length originally, size now, j: %5d, %5d\n", length, myCAM.read_fifo_length());

  fclose(fp);
    
  //fclose(chunkfile);
  
  myCAM.flush_fifo();
  
  myCAM.cs_high();
  cam_cs = 1;
  return 1;
  
}

void disable_camera()
{
    /*
    CAM_SHUTDOWN = 1;
    cam_en = 0;
    */
}

void camera_enabled(uint8_t enable) // 0 to disable, 1 to enable
{
    if(enable == 1){
        if(camera_is_on){
            camera_timeout.detach();
        }
        camera_timeout.attach(disable_camera, 30s);
        if(!camera_is_on)
        {
            myCAM.cs_low();
            cam_cs = 0;
            myCAM.clear_bit(ARDUCHIP_GPIO, LOW_POWER_MODE);
            myCAM.clear_bit(ARDUCHIP_GPIO, FIFO_PWRDN_MASK);
            
            ThisThread::sleep_for(100ms);
            
            myCAM.cs_high();
            cam_cs = 1;
        }
    }else{
        if(camera_is_on){
            myCAM.cs_low();
            cam_cs = 0;

            myCAM.set_bit(ARDUCHIP_GPIO, LOW_POWER_MODE);
            myCAM.set_bit(ARDUCHIP_GPIO, FIFO_PWRDN_MASK);

            myCAM.cs_high();
            cam_cs = 1;
        }
    }
    
    camera_is_on = enable;
}

void camera_should_disable_stream_mod()
{
    CLOSE_STREAM = 0;
}


void camera_enable_stream_mode()
{
    if(stream_on)
    {
        stream_timeout.detach();
    }
    stream_timeout.attach(camera_should_disable_stream_mod, 15s);
    if(stream_on) return;

    myCAM.OV2640_set_JPEG_size(STREAM_RESOLUTION);

    if(!stream_on)
    {
        ThisThread::sleep_for(100ms);
    }

    stream_on = 1;
}


void camera_disable_stream_mode()
{
    if(!stream_on) return;

    myCAM.OV2640_set_JPEG_size(DEFAULT_RESOLUTION);

    stream_on = 0;
}


int get_blocks(int block_num, char* filename, char* block_buffer)
{
      std::ifstream ifs(filename, std::ifstream::binary);
      if(!ifs)
      {
          _error("Can't open file for reading: %s\n\r", filename);
          return FILE_OPEN_FAILED;
      }
        ifs.seekg(block_num * 53, ios_base::beg);

        ifs.read(block_buffer, 53);
        ifs.close();

      
      _debug("Block:\n\r");
      for(int k=0; k<53; k++)
      {
          pc_printf("%04x",(char)block_buffer[k]);
      }
      pc_printf("\n\r");

      

      return 0;
}

int getCamId()
{
    
    int err = 0;
    uint8_t vid = 0;
    uint8_t pid = 0;

    feed_the_dog();
    
    myCAM.cs_high();
    cam_cs = 1;
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if  ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    {
        _error("Can't find OV2640 module! vid = 0x%x pid = 0x%x\r\n",vid, pid);
        //while(1);
        err = -2;
    }

    
    myCAM.cs_low();
    cam_cs = 0;
    
    return err;
}

int cam_test_SPI()
{
    cam_cs = 0;
    myCAM.cs_low();
    ThisThread::sleep_for(100ms);
    feed_the_dog();
    while(1)
    {
      //Check if the ArduCAM SPI bus is OK
      //myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
      myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
      uint8_t mtemp = myCAM.read_reg(ARDUCHIP_TEST1);
      
      if (mtemp != 0x55)
      {
        _error("ACK CMD SPI interface Error!\n\r");
        _error("SPI error: %d\n\r", mtemp);
        ThisThread::sleep_for(1s);
        //continue;
        return -1;
      } else {
        break;
      }
    }
    
    cam_cs = 1;
    myCAM.cs_high();
    
    return 0;
}

void cam_set_mode()
{
    myCAM.cs_low();
    cam_cs = 0;
    
    myCAM.set_mode(MCU2LCD_MODE);
    myCAM.cs_high();
    cam_cs = 1;
}   

void cam_reset_cpld()
{
    
    //Reset the CPLD
    myCAM.write_reg(0x07, 0x80);
    ThisThread::sleep_for(200ms);
    myCAM.write_reg(0x07, 0x00);
    ThisThread::sleep_for(200ms);

}

void cam_setup()
{
    myCAM.set_format(JPEG);
    myCAM.InitCAM();
    // No need to set the size, InitCam sets it
    // myCAM.OV2640_set_JPEG_size(OV2640_320x240);
    myCAM.clear_fifo_flag();
    // is this the trigger for the non copyable error?
    myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
}

void set_SPI1_for_camera(void)
{
    // For the LED driver
    camspi.frequency(8000000);
}

void camera_capture_on(void)
{
    if(!camera_is_on){
        camera_enabled(1);
    }
    if(CAPTURE_AND_STREAM)
    {   
        camera_enable_stream_mode();
    }
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
}

int camera_setup(void)
{
    _debug("Loading camera module... \n\r");
    cam_cs = 0;
    cam_en = 1;
    myCAM.cs_low();
    int err = 0;
    feed_the_dog();
    cam_reset_cpld();  
    feed_the_dog();  
    camera_enabled(1);
    err = cam_test_SPI();
    err = getCamId();

    if( !err )
    {
        cam_setup();
        _ack("OK\n\r");


      //  ISRCAMthread.start(callback(CAM_read_thread));

    }
    else {
        _error("FAIL!\n\r");
    }
    camera_timeout.attach(disable_camera, 10s);

    cam_cs = 1;
    myCAM.cs_high();
    //cam_en = 0;

    
    return err;
}

void reset_cam()
{    
    camera_power_off();
    ThisThread::sleep_for(200ms);
    feed_the_dog();
    camera_power_on();

    camera_setup();
}

void camera_power_off()
{
    //cam_en = 0;
}

void camera_power_on()
{
    cam_en = 1;
    feed_the_dog();
    ThisThread::sleep_for(200ms);
}

void query_image_with_index(int index, char img_name[])
{
    sprintf(img_name, "image%d.jpg", index);
}

/*********** COMMAND INTERFACE **************/

void LIST_N(uint8_t cmd, char* _params)
{
    char most_recent_images[20][50];
    memset(most_recent_images,0,sizeof(most_recent_images));

    int n = str2int(_params);
    n < 1 ? n = 1 : n;

    n = n_latest_files(n, most_recent_images);

    if (n > 0)
    {
        _debug("%d most recent images:\n\r", n);

        int size_flatted = 0;
        for(int l=0; l<n; l++)
        {
            _debug("%s\n\r", most_recent_images[l]);

            size_flatted += strlen(most_recent_images[l]) + 1;
        }
    
        sendPacket(CAM, 1, (char*)most_recent_images[0], size_flatted * sizeof(char) );
    }
    else {
        _error("IO Error\n\r");
    }
}
void TAKE_PIC(uint8_t cmd, char* _params)
{
    //char* blocks_response = "take_pic";

    __CAM_CAPTURE_FLAG = 1;
    CAM_ENABLED = 1;
    
    //sendPacket(CAM, 4, blocks_response, strlen(blocks_response));
}

void STREAM_PIC(uint8_t cmd, char* _params)
{
    CAM_ENABLED = 1;
    __CAM_CAPTURE_FLAG = 1;
    CAPTURE_AND_STREAM = 1;
}

#define BLOCK_BURST_COUNT 20

void BLOCKS(uint8_t cmd, char* _params)
{
    char img_name[50];
    char img_path[64];
    char block_data_buffer[53];

    memset(block_data_buffer, 0, sizeof(block_data_buffer));
    memset(img_name, 0, sizeof(img_name));
    memset(img_path, 0, sizeof(img_path));

    int img_index = str2int(strtok((char*)_params, WHITESPACE));
    query_image_with_index(img_index, img_name);

    sprintf(img_path, "/fs/mydir/%s", img_name);

    int block_indices[BLOCK_BURST_COUNT];
    for(int i = 0; i < BLOCK_BURST_COUNT; i++)
    {
        char* aa = strtok(NULL, WHITESPACE);
        if(aa == NULL)
        {
            block_indices[i] = block_indices[i-1] + 1;
        }else{
            block_indices[i] = strtol(aa, NULL, 10);
        }
    }


    std::ifstream ifs(img_path, std::ifstream::binary);
    if(!ifs)
    {
        _error("Can't open file for reading: %s\n\r", img_path);
        return;
    }
    ifs.seekg(0, ios_base::beg);
    int lastIndex = 0;
    
    for(int i = 0; i<BLOCK_BURST_COUNT; i++)
    {
        if(lastIndex != block_indices[i]-1)
        {
            if(lastIndex == 0) lastIndex -= 1;
            ifs.seekg((block_indices[i] - lastIndex - 1) * 53, ios_base::cur);
        }
        ifs.read(block_data_buffer, 53);
        sendPacketRealTime(CAM, 3, block_data_buffer, 53, block_indices[i]);
        pc_printf("Sent block %d\r\n", block_indices[i]);
        feed_the_dog();
        if(!ifs){
            break;
        }
        memset(block_data_buffer, 0, sizeof(block_data_buffer));
        lastIndex = block_indices[i];
        ThisThread::sleep_for(50ms);


    }

    ifs.close();

}

void NUM_BLOCKS(uint8_t cmd, char* _params)
{

    char img_name[50];
    feed_the_dog();
    query_image_with_index(str2int(_params), img_name);

    int c = get_size_in_chunks(img_name);

    /*if( c > 0 )
        _debug("Num blocks: %d\n\r", c);*/

    char block_num[3];


    sprintf(block_num, "%d", c);

    feed_the_dog();
        
    sendPacket(CAM, 2, block_num, strlen(block_num));
}

void CAM_RESET(uint32_t cmd, char *_params)
{
    reset_cam();
    _message("cam_rst", CAM, 5);
}
void CAM_PING(uint32_t cmd, char *_params)
{
    // Deprecated
    /*char ping[2];
    char ping_response[5];
    memset(ping,0,sizeof(ping));
    memset(ping_response,0,sizeof(ping_response));
    
    sprintf(ping, "%d", cmd);
    
    sendPacket(CAM, 6, ping_response, strlen(ping_response));*/
}
void CAM_LATEST(uint32_t cmd, char *_params)
{
    char most_recent_image[50];
    memset(most_recent_image,0,sizeof(most_recent_image));

    int n = 0;
    n = latest_file(most_recent_image);

    sendPacket(CAM, 7, most_recent_image, strlen(most_recent_image));
}