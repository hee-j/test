#ifndef __CMD_INTERPRETER_H
#define __CMD_INTERPRETER_H

#include "mbed.h"
#include <string>

void cmd_OBC(uint32_t cmd, char *_params);
void cmd_CAM(uint32_t cmd, char *_params);
void cmd_GPS(uint32_t cmd, char *_params);
void cmd_BMP(uint32_t cmd, char *_params);
void cmd_IMU(uint32_t cmd, char *_params);
void cmd_RF(uint32_t cmd, char *_params);
void cmd_BUZZER(uint32_t cmd, char *_params);
void cmd_EPS(uint32_t cmd, char *_params);
void cmd_LED(uint32_t cmd, char *_params);
void cmd_General(uint32_t cmd, char* _params);
void cmd_GS(uint32_t cmd, char* _params);
void cmd_DEBUG(uint32_t cmd, char* _params);
void cmd_BT(uint32_t cmd, char* _params);
void cmd_SETTINGS(uint32_t cmd, char* _params);
void cmd_FILETRANSFER(uint32_t cmd, char* _params, int data_len);
void cmd_EXTERNAL_PAYLOAD(uint32_t cmd, char* _params, uint16_t data_len);

int parse_cmd(char *bts, int length);
void interpret(uint32_t target, uint32_t cmd, char *_params, int data_len);

void _RESET();

#endif