#ifndef MYALLOC_H
#define MYALLOC_H

#include "config.h"

// typedef unsigned int u32;

void myalloc_init();
void* myalloc(u32 sizeofbyte);
void* myrealloc(void* ptr, u32 sizeofbyte);
void myfree(void* ptr);

#endif // !MYALLOC_H
