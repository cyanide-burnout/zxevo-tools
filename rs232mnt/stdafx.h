#pragma once

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/mman.h>

#include "types.h"
#include "fifo.h"

#define min(a, b)  (a) < (b) ? (a) : (b)
#define TEXT(s)    const_cast<char*>(s)
#define _TCHAR     const char
#define DWORD      uint32_t
#define HANDLE     int
#define _tmain     main

extern "C"
{
  int OpenSerialPort(const char* device, int speed, int flags);

  void SetInterruptionMask();
  int CheckInterruption();

  void Sleep(int interval);
  void ReadFile(int handle, void* buffer, uint32_t size, uint32_t* count, void* unused);
  void WriteFile(int handle, void* buffer, uint32_t size, uint32_t* count, void* unused);
}

bool SetDiskImage(int number, const char* path, uint8_t** buffer, uint32_t size);
