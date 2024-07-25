#include "led.h"
#include "../utilities/hw_config.h"
#include "settings.h"
#include <chrono>
using namespace std::chrono;

#if defined(LED_DRIVER_IS_TLC)
#include "TLC59711/TLC59711.h"
TLC59711 led_driver(1, LED_DRIVER_SPI_SCLK, LED_DRIVER_MOSI);
#elif defined(LED_DRIVER_IS_STP)
#include "STP16CPC.h"
STP16CPC led_driver(1, LED_DRIVER_SPI_SCLK, LED_DRIVER_MOSI);
#endif

//DigitalOut led_cs(LED_DRIVER_CS);

void led_show_update(void);


uint8_t LED_SHOW_ACTIVE = 0;
float led_show_delay = 0.5;
Timeout led_update_timeout;
uint8_t NEEDS_A_TIMEOUT = 0;
uint8_t REQUIRES_AN_UPDATE = 0;

void setColors(int led, int *colors)
{
    if(led == 1)
    {
        int g = colors[1];
        colors[1] = colors[2];
        colors[2] = g;
    }
    led_driver.setLED(led, colors[0], colors[1], colors[2]);
}

void tlc_update()
{
    led_driver.write();
}


void leds_init()
{
    led_driver.setLED(0, 0, 0, 0);
    led_driver.write();
}

void parse_color_code(int colorCode, int *colors)
{
    if(sizeof(colors) < 3) return;

    if(colorCode == 0)
    {
        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 0;
        return;
    } 
    if(colorCode == 1)
    {
        colors[0] = 10000;
        colors[1] = 0;
        colors[2] = 0;
        return;
    }    
    if(colorCode == 2)
    {
        colors[0] = 0;
        colors[1] = 10000;
        colors[2] = 0;
        return;
    }
    if(colorCode == 3)
    {
        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 10000;
        return;
    }
    if(colorCode == 4)
    {
        colors[0] = 10000;
        colors[1] = 10000;
        colors[2] = 10000;
        return;
    }
    
    if(colorCode == 5)
    {
        colors[0] = 0;
        colors[1] = 10000;
        colors[2] = 10000;
        return;
    }
    
    if(colorCode == 6)
    {
        colors[0] = 10000;
        colors[1] = 0;
        colors[2] = 10000;
        return;
    }
    
    if(colorCode == 7)
    {
        colors[0] = 10000;
        colors[1] = 10000;
        colors[2] = 0;
        return;
    }
}

void LED_STOP(char* _params)
{
    LED_SHOW_ACTIVE = 0;
    LED_LIT((char *) "0 0");
}


void LED_LIT(char* _params)
{
    int _LED = _params[0]-'0';
    int _color = _params[2]-'0';
    char answer[32];

    int colors[3];
    memset(colors, 0, sizeof(colors)); 
    parse_color_code(_color, colors);

    // LED 0 means all colors
    if(_LED == 0)
    {
        for(int i = 0; i<3; i++)
        {
            setColors(i, colors);
        }
    }else{
        setColors(_LED-1, colors);
    }
    tlc_update();
}


void LED_START  (char* _params){
    LED_SHOW_ACTIVE = 1;
    REQUIRES_AN_UPDATE = 1;
    int del = str2int(_params);
    led_show_delay = del/1000.0;
}

void LED_BLINK   (char* _params){

}

void led_update_flag_set(void)
{
    REQUIRES_AN_UPDATE = 1;
}

void led_thread(void)
{
    if(REQUIRES_AN_UPDATE)
    {
        led_show_update();
        REQUIRES_AN_UPDATE = 0;
    }
}

void led_show_update(void)
{
    if(!LED_SHOW_ACTIVE) return;
    int colors[3];
    uint8_t led_num = rand()%3;
    uint8_t color = rand()%7+1;
    parse_color_code(color, colors);
    setColors(led_num, colors);
    tlc_update();
    led_update_timeout.attach(led_update_flag_set, chrono::duration_cast<chrono::milliseconds>(chrono::duration<float>(led_show_delay)));
}
