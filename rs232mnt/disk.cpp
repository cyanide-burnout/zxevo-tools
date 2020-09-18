#include <stdint.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>

#include "DiskImage.h"

inline const char* GetTemporaryFolder()
{
  const char* value;
  struct stat status;

  value = getenv("TMP");

  if ((value == NULL) &&
      (stat("/tmp", &status) == 0) &&
      (S_ISDIR(status.st_mode)))
  {
    // Use /tmp as a temporary folder
    value = "/tmp";
  }

  if (value == NULL)
  {
    // Use current folder when empty
    value = ".";
  }

  return value;
}

bool SetDiskImage(int number, const char* path, uint8_t** buffer, uint32_t size)
{
  const char* file;
  const char* extension;
  const char* temporary;

  int handle;
  struct stat status;
  char name[PATH_MAX];

  TDiskImage image;

  file = path;

  if ((path == NULL) ||
      (extension = strrchr(path, '.')) &&
      (strcmp(extension, ".trd") != 0))
  {
    file      = name;
    temporary = GetTemporaryFolder();
    snprintf(name, PATH_MAX, "%s/rs232mnt-%d.trd", temporary, number);
  }

  printf("Using %s for drive %d\n", file, number);
  handle = open(file, O_CREAT | O_RDWR, 0666);

  if (handle < 0)
  {
    printf("Error openning file (%i)\n", errno);
    close(handle);
    return false;
  }

  if ((path != NULL) &&
      (path != file))
  {
    printf("Converting image %s ..\n", path);
    ftruncate(handle, 0);
    image.Open(const_cast<char*>(path), true);
    image.writeTRD(handle);
  }

  if ((fstat(handle, &status) == 0) &&
      (status.st_size == 0))
  {
    printf("Formating image ..\n");
    image.formatTRDOS(80, 2);
    image.writeTRD(handle);
    fstat(handle, &status);
  }

  /*
  if ((fstat(handle, &status) == 0) &&
      (status.st_size != size))
  {
    ftruncate(handle, size);
    status.st_size = size;
  }
  */

  *buffer = (uint8_t*)mmap(NULL, status.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

  if (*buffer == (uint8_t*)MAP_FAILED)
  {
    printf("Error creating memory map for %s\n", file);
    close(handle);
    return false;
  }

  close(handle);
  return true;
}
