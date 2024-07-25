#pragma once
#ifndef __WD_H_
#define __WD_H_

#include "mbed.h"

// HARWARE WATCHDOG

void    feed_the_dog(void);
void    wd_init(void);

// SOFTWARE WATCHDOG (ST iwdg)
// Not implemented
/*
class SWWatchdog {
public:
    /// Create a Watchdog object
    ///
    /// example:
    /// @code
    /// Watchdog wd;    // placed before main
    /// @endcode
    SWWatchdog();
    
    /// Configure the timeout for the Watchdog
    ///
    /// This configures the Watchdog service and starts it. It must
    /// be serviced before the timeout, or the system will be restarted.
    ///
    /// example:
    /// @code
    ///     ...
    ///     wd.Configure(1.4);  // configure for a 1.4 second timeout
    ///     ...
    /// @endcode
    ///
    /// @param[in] timeout in seconds, as a floating point number
    /// @returns none
    ///
    void Configure(float timeout);
    
    /// Service the Watchdog so it does not cause a system reset
    ///
    /// example:
    /// @code
    ///    wd.Service();
    /// @endcode
    /// @returns none
    void Service();
    
    /// WatchdogCausedReset identifies if the cause of the system
    /// reset was the Watchdog
    ///
    /// example:
    /// @code
    ///    if (wd.WatchdogCausedReset())) {
    /// @endcode
    ///
    /// @returns true if the Watchdog was the cause of the reset
    bool WatchdogCausedReset();
private:
    bool wdreset;
};*/

#endif // __WD_H_