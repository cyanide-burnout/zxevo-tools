#include <stdint.h>
#include <string.h>

#include "DiskImage.h"

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
              // Unsupported data format or CRC
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