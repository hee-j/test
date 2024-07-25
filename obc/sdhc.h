#ifndef __K_SDHC_H_
#define __K_SDHC_H_

#include "mbed.h"

#include <stdio.h>
#include <errno.h>

#include "hw_config.h"
#include "logging.h"

// Block devices
//#include "SPIFBlockDevice.h"
//#include "DataFlashBlockDevice.h"
#include "SDBlockDevice.h"
//#include "HeapBlockDevice.h"
 
// File systems
 
//#include "LittleFileSystem.h"
#include "FATFileSystem.h"

typedef struct m_File {
    const char* filepath;
    FILE *f;
} mFile;

int filesystem_mount(void);
uint8_t is_mounted(void);

int open_file(const char* filepath, mFile *myFile);
int sd_cleanup(void);
int deinit_bd();
int sd_unmount();

int display_root_directory(void);
int close_file(mFile *myFile);
int display_file(mFile *myFile);

// Unit tests
int sd_card_unit_test(void);

int sd_increment(mFile *myFile);

#endif