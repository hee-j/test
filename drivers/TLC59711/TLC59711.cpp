#include "mbed.h"
#include "stdint.h"
#include "TLC59711.h"
#include "hw_config.h"

DigitalOut TLC_cs(LED_DRIVER_CS);

void led_disable()
{
    TLC_cs = 1;
}

void led_enable()
{
    TLC_cs = 0;
}

TLC59711::TLC59711(uint8_t number, PinName SCLK, PinName MOSI) : n(number), spi(MOSI, NC, SCLK)
{
    numdrivers = n;   
    TLC_cs = 0;
    // Configure SPI to 8 bits and SPI_SPEED  
    spi.format(8, 0);
    spi.frequency(SPI_SPEED);

    TLC_cs = 1;
    
    BCr = BCg = BCb = 0x7F;
    pwmbuffer = (uint16_t *)calloc(2, 12*n);
}

void TLC59711::write(void) {
  uint32_t command;
  TLC_cs = 1;
  // Magic word for write
  command = 0x25;
  command <<= 5;
  //OUTTMG = 1, EXTGCK = 0, TMGRST = 1, DSPRPT = 1, BLANK = 0 -> 0x16
  command |= 0x16;
  command <<= 7;
  command |= BCr;
  command <<= 7;
  command |= BCg;
  command <<= 7;
  command |= BCb;
  for (uint8_t n=0; n<numdrivers; n++) {
    spi.write(command >> 24);
    spi.write(command >> 16);
    spi.write(command >> 8);
    spi.write(command);
    // 12 channels per TLC59711
    for (int8_t c=11; c >= 0 ; c--) {
      // 16 bits per channel, send MSB first
      spi.write(pwmbuffer[n*12+c]>>8);
      spi.write(pwmbuffer[n*12+c]);
    }
  }
  TLC_cs = 0;
}

void TLC59711::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan > 12*numdrivers) return;
  pwmbuffer[chan] = pwm;  
}

void TLC59711::setLED(uint8_t lednum, uint16_t r, uint16_t g, uint16_t b) {
  setPWM(lednum*3, r);
  setPWM(lednum*3+1, g);
  setPWM(lednum*3+2, b);
}