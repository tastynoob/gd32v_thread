#ifndef INIT_C
#define INIT_C

#include <gd32vf103.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "riscv_encoding.h"
#include "gd32vf103_libopt.h"
#include "lcd.h"
#include "thread.h"




#define _DEBUG_


#ifdef _DEBUG_
#define log printf
#else
#define log
#endif

extern volatile char str_read[];





void Init();


uint32_t avaliable();
uint32_t readAll();
void buffClear();
uint32_t readLine();
uint32_t readbyte2();
uint32_t readch();

void print_char(char ch);

























#endif