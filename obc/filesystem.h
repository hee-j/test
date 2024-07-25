#pragma once
#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_

#include "mbed.h"
#include "../utilities/serial/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "legacy_comm.h"
#include "timers.h"
#include "m_util.h"
#define FILESYSTEM_NOMINAL 0
#define FILESYSTEM_ERROR_NO_SDCARD 1
#define FILESYSTEM_ERROR_WRITE_FAIL 2
#define FILESYSTEM_ERROR_READ_FAIL 2

int test_filesystem();

int n_latest_files(unsigned int n, char most_recent_images[20][50]);
int get_size_in_chunks(char* image_name);
int latest_file(char* most_recent_image);
int INITIATE_UPLINK(char *params);
void UPLINK(char *params, int data_len);
void FINALIZE_UPLINK();
void UPLINK_FNV();
void UPLINK_STATUS();
void PRINT_FILE();
uint8_t DELETE_UPLINK_FILE();
void NEXT_BLOCKS();


int readFilesize(int fd);
int readFilesize(char *filepath);

#endif