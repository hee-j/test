#pragma once

#ifndef __BMP_H_
#define __BMP_H_

    #include "mbed.h"
    #include "settings.h"
    #include <string.h>

    #include "hw_config.h"
    //#include "logging.h"
    #include "flags.h"
    #include "submodule.h"

    #include "legacy_comm.h"

    int env_init(void);
    float env_get_temp();
    float env_get_pressure();

    // Cmd interface
    void    ENV_TEMP();
    void    ENV_PRESS();
    void    ENV_ALL();

#endif