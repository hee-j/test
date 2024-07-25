#include "wd.h"
#include "logging.h"

// HARWARE WATCHDOG

DigitalOut      dog(PD_2);
DigitalOut      led(PA_0);
int dog_state   = 0;

void wd_init(void)
{
    dog = 0;
    feed_the_dog();

    _debug("Hardware watchdog set up.\n\r");
}

void feed_the_dog(void)
{
    if(dog_state == 0)
    {
      dog_state = 1;   
    }else{
      dog_state = 0;  
    }
    dog = dog_state;   
    led = dog_state;
}

// SOFTWARE WATCHDOG (ST iwdg)

/// Watchdog gets instantiated at the module level
/*SWWatchdog::SWWatchdog() {
#ifdef LPC    
    wdreset = (LPC_WDT->WDMOD >> 2) & 1;    // capture the cause of the previous reset
#endif
#ifdef ST_NUCLEO    
        wdreset = false; 
#endif
 
}

/// Load timeout value in watchdog timer and enable
void SWWatchdog::Configure(float timeout) {
#ifdef LPC    
    LPC_WDT->WDCLKSEL = 0x1;                // Set CLK src to PCLK
    uint32_t clk = SystemCoreClock / 16;    // WD has a fixed /4 prescaler, PCLK default is /4
    LPC_WDT->WDTC = (uint32_t)(timeout * (float)clk);
    LPC_WDT->WDMOD = 0x3;                   // Enabled and Reset
#endif   
#ifdef ST_NUCLEO
    // see http://embedded-lab.com/blog/?p=9662    
    #define LsiFreq (45000)
    
    uint16_t PrescalerCode;
    uint16_t Prescaler;
    uint16_t ReloadValue;
    float Calculated_timeout;
    
    if ((timeout * (LsiFreq/4)) < 0x7FF) {
        PrescalerCode = IWDG_PRESCALER_4;
        Prescaler = 4;
    }
    else if ((timeout * (LsiFreq/8)) < 0xFF0) {
        PrescalerCode = IWDG_PRESCALER_8;
        Prescaler = 8;
    }
    else if ((timeout * (LsiFreq/16)) < 0xFF0) {
        PrescalerCode = IWDG_PRESCALER_16;
        Prescaler = 16;
    }
    else if ((timeout * (LsiFreq/32)) < 0xFF0) {
        PrescalerCode = IWDG_PRESCALER_32;
        Prescaler = 32;
    }
    else if ((timeout * (LsiFreq/64)) < 0xFF0) {
        PrescalerCode = IWDG_PRESCALER_64;
        Prescaler = 64;
    }
    else if ((timeout * (LsiFreq/128)) < 0xFF0) {
        PrescalerCode = IWDG_PRESCALER_128;
        Prescaler = 128;
    }
    else {
        PrescalerCode = IWDG_PRESCALER_256;
        Prescaler = 256;
    }
    
    // specifies the IWDG Reload value. This parameter must be a number between 0 and 0x0FFF.
    ReloadValue = (uint32_t)(timeout * (LsiFreq/Prescaler));
    
    Calculated_timeout = ((float)(Prescaler * ReloadValue)) / LsiFreq;
    _debug("WATCHDOG set with prescaler:%d reload value: 0x%X - timeout:%f\n",Prescaler, ReloadValue, Calculated_timeout);
    
    IWDG->KR = 0x5555; //Disable write protection of IWDG registers      
    IWDG->PR = PrescalerCode;      //Set PR value      
    IWDG->RLR = ReloadValue;      //Set RLR value      
    IWDG->KR = 0xAAAA;    //Reload IWDG      
    IWDG->KR = 0xCCCC;    //Start IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf       
#endif
 
    Service();
}

/// "Service", "kick" or "feed" the dog - reset the watchdog timer
/// by writing this required bit pattern
void SWWatchdog::Service() {
#ifdef LPC    
    LPC_WDT->WDFEED = 0xAA;
    LPC_WDT->WDFEED = 0x55;
#endif    
#ifdef ST_NUCLEO
    IWDG->KR = 0xAAAA;         //Reload IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf
#endif
}

/// get the flag to indicate if the watchdog causes the reset
bool SWWatchdog::WatchdogCausedReset() {
    return wdreset;
}*/