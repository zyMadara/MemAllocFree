#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void MemInit(void);
extern void* MemAlloc(uint16_t size);
extern void MemFree(void* p);