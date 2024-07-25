#include "timers.h"
#include <chrono>


Timer time_since_boot;

void start_boot_timer()
{
    time_since_boot.start();
}

unsigned int ms_since_boot()
{
    return chrono::duration_cast<chrono::milliseconds>(time_since_boot.elapsed_time()).count();
}