#ifndef STP16CPC_H
#define STP16CPC_H

#include "mbed.h"

/**
  * SPI speed used by the mbed to communicate with the STP24DP05
  * The STP24DP05 supports up to ????
  */
#define SPI_SPEED 8000000

/**
  *  Using the TLC5 class to control an LED:
  *  @code
  *  
  *  
  *  
  *  // Create the TLC5711 instance
  *  TLC59711 tlc(1, p7, p5);
  *  
  *  int main()
  *  {   
  *      
  *      while(1)
  *      {
  *         // Led1 -> R0
  *         tlc.setLED(0, 65535, 0, 0);
  *         tlc.write( );
  *         tlc.setLED(1, 0, 0, 0);
  *         tlc.write( );
  *         tlc.setLED(2, 0, 0, 0);
  *         tlc.write( );
  *         tlc.setLED(3, 0, 0, 0);
  *         tlc.write( );
  *         wait( 1 );
  *  
  *      }
  *  }
  *  @endcode
  */

 class STP16CPC
{

public:/**
      *  Set up the STP24DP05
      *  @param SCLK - The SCK pin of the SPI bus
      *  @param MOSI - The MOSI pin of the SPI bus
      *  @param number - The number of STP24DP05s 
      */
  
    STP16CPC(uint8_t number, PinName SCLK, PinName MOSI);

  void setPWM(uint8_t chan, uint16_t pwm);
  void setLED(uint8_t lednum, uint16_t r, uint16_t g, uint16_t b);
  void write(void);    
    void testwrite(uint16_t val);
        
  uint8_t n;
  
  private:
  
  SPI spi;
  
  uint16_t *pwmbuffer;

  uint8_t BCr, BCg, BCb;
  int8_t numdrivers;

};
 
#endif