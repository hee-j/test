#include "STP16CPC.h"
#include "../hw_config.h"
DigitalOut STP_cs(LED_DRIVER_CS);
uint16_t led_status = 0;

//STP16CPC stp(1, LED_DRIVER_SPI_SCLK, LED_DRIVER_MOSI);
/*
void led_driver_set_colors(uint8_t led, int *colors)
{
    stp.setLED(led, colors[0], colors[1], colors[2]);
}


void led_driver_init(void)
{
    led_cs = 1;
}

void led_driver_update(void)
{
    stp.write();
}
*/
STP16CPC::STP16CPC(uint8_t number, PinName SCLK, PinName MOSI) : n(number), spi(MOSI, NC, SCLK)
{
    numdrivers = n;   
    STP_cs = 0;
    spi.format(8, 0);
    spi.frequency(SPI_SPEED);
}

void STP16CPC::write(void) {// void S¨ååååååååååååååååååååååååååååååååååå´+´+´+TP16CPC::write(void) {
  
  STP_cs = 1;
  spi.write(led_status >> 8);
  spi.write(led_status);
  STP_cs = 0;

}

uint8_t toBit(uint16_t val)
{
    if(val > 127) return 1;
    return 0;
}

void STP16CPC::setLED(uint8_t lednum, uint16_t r, uint16_t g, uint16_t b) {
 //   printf("Set %d to %d, %d, %d\r\n", lednum, r, g, b);

    // clear all bits from the correct led
    uint16_t reset_mask = 0xFFFF & ~(0x07<<(3*lednum));
    led_status &= reset_mask;

    // re-set necessary bits
    led_status |= toBit(r)<<(lednum*3);
    led_status |= toBit(g)<<(lednum*3+1);
    led_status |= toBit(b)<<(lednum*3+2);
}

void STP16CPC::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan > 12*numdrivers)
    return;
  pwmbuffer[chan] = pwm;
}

void STP16CPC::testwrite(uint16_t val)
{
    spi.write(val);
}