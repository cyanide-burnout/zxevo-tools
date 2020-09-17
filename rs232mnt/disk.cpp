#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "DiskImage.h"

inline bool CheckFileExtension(const char* value1, const char* value2)
{
  // Magic! :)
  return !((*(uint32_t*)value1 ^ *(uint32_t*)value2) & 0x1f1f1f1f);
}

bool LoadDiskImage(const char* path, uint8_t* buffer)
{
  TDiskImage image;
  VGFIND_SECTOR data;

  int side;
  int track;
  int sector;

  image.AddBOOT = false;
  image.Open(const_cast<char*>(path), true);

  if (image.DiskPresent)
  {
    for (track = 0; track <= image.MaxTrack; track ++)
    {
      for (side = 0; side <= image.MaxSide; side ++)
      {
        for (sector = 1; sector <= 16; sector ++)
        {
          if (image.FindSector(track, side, sector, &data))
          {
            if (!data.CRCOK ||
                !data.vgfa.CRCOK ||
                (data.SectorLength != 256))
            {
              // Unsupported data format or CRC error
              return false;
            }

            memcpy(buffer, data.SectorPointer, data.SectorLength);
            buffer += data.SectorLength;

            continue;
          }

          memset(buffer, '*', 256);
          buffer += 256;
        }
      }
    }
  }

  return image.DiskPresent;
}

bool SaveDiskImage(const char* path, uint8_t* buffer)
{
  int handle;
  const char* extension;

  TDiskImage image;
  VGFIND_SECTOR data;

  int side;
  int track;
  int sector;

  struct stat status;

  image.formatTRDOS(80, 2);

  for (track = 0; track <= image.MaxTrack; track ++)
  {
    for (side = 0; side <= image.MaxSide; side ++)
    {
      for (sector = 1; sector <= 16; sector ++)
      {
        image.FindSector(track, side, sector, &data);
        memcpy(data.SectorPointer, buffer, data.SectorLength);
        buffer += data.SectorLength;
        image.ApplySectorCRC(data);
      }
    }
  }

  handle    = open(path, O_CREAT | O_RDWR | O_TRUNC, 0666);
  extension = strrchr(path, '.');

  if ((handle < 0) ||
      (extension == NULL) ||
      (strlen(extension) != 4))
  {
    // Cannot open a file for write
    close(handle);
    return false;
  }

  if (!CheckFileExtension(extension, ".TRD")) image.writeTRD(handle);
  if (!CheckFileExtension(extension, ".SCL")) image.writeSCL(handle);
  if (!CheckFileExtension(extension, ".FDI")) image.writeFDI(handle);
  if (!CheckFileExtension(extension, ".UDI")) image.writeUDI(handle);
  if (!CheckFileExtension(extension, ".TD0")) image.writeTD0(handle);
  if (!CheckFileExtension(extension, ".FDD")) image.writeFDD(handle);

  fstat(handle, &status);
  close(handle);

  return status.st_size;
}
