#ifndef __CAMERA_H
#define __CAMERA_H

#include "mbed.h"
#include <iostream> 
#include <sstream> 
#include "ov2640/ArduCAM.h"
#include "ov2640/ov2640_regs.h"
#include "sdhc.h"

#include "hw_config.h"
#include "submodule.h"
#include "logging.h"
#include "wd.h"

#include "flags.h"

#include "filesystem.h"
#include "errors.h"
#include "wd.h"

// Rehaul
#include "legacy_comm.h"

int     camera_setup            (void);
void    camera_capture_on       (void);
int     camera_save_initiated   (char *filepath);
void    CAM_stream_pic          (void);
void    camera_enabled          (uint8_t enable);
void    camera_disable_stream_mode (void);
void    CAM_read_thread         (void);

void camera_power_off(void);
void camera_power_on(void);
// SPI also in use for LED driver
void    set_SPI1_for_camera(void);

// COMMAND INTERFACE

void LIST_N         (uint8_t cmd, char* _params);
void TAKE_PIC       (uint8_t cmd, char* _params);
void BLOCKS         (uint8_t cmd, char* _params);
void NUM_BLOCKS     (uint8_t cmd, char* _params);
void CAM_RESET      (uint32_t cmd, char *_params);
void CAM_PING       (uint32_t cmd, char *_params);
void CAM_LATEST     (uint32_t cmd, char *_params);
void STREAM_PIC     (uint8_t cmd, char *_params);

#define MAX_FIFO_SIZE 512 * 1024
#define LARGE_IMG_BUF_SIZE 10240


#define WHITESPACE "\n\r\t "

#endif // __CAMERA_H