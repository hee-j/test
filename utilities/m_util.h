#ifndef __M_UTIL_H_
#define __M_UTIL_H_

#include "mbed.h"
#include <string>

int max(int a, int b);
int min(int a, int b);

long chartobin(char c);
int isDigit(char c);
int str2int(const char *s);
int int2bool(int i);
void print_memory_info();

uint32_t    ufnv(char *bytes, int str_len);
int         checkFNV(char *data, int length);

void printPacket(char *packet, int packetLength);
int remove_duplicate_elements(int arr[], int n);

void system_rst();


#endif