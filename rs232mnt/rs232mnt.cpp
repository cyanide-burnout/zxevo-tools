#include "stdafx.h"

#define TRD_SZ  256*16*255
#define BAUD    115200
#define TOKEN1  0xAA
#define TOKEN2  0x55
#define ANS1    0xCC
#define ANS2    0xEE

enum OPCODE : U8
{
  OP_RD = 5,
  OP_WR = 6
};

enum STATE
{
  ST_IDLE = 0,
  ST_RECEIVE_START_TOKEN2,
  ST_RECEIVE_HEADER,
  ST_RECEIVE_DATA
};

#pragma pack (push, 1)
typedef struct
{
  U8      drv;
  OPCODE  op;
  U8      trk;
  U8      sec;
  U8      crc;
} REQ;

typedef struct
{
  U8      ack[2];
  U8      data[256];
  U8      crc;
} SECT;
#pragma pack (pop)

U8* img[4]; // U8 img[4][TRD_SZ];
int baud = BAUD;
bool _log = false;
bool slow = false;
_TCHAR* cport = TEXT("/dev/ttyUSB0");
_TCHAR* trd[4];
U8 drvs = 0;
U8 fifo_in_buf[512];
U8 uart_in_buf[512];

void print_help()
{
  printf("RS-232 VDOS Mounter,  (c) 2013-2021 TS-Labs inc.\n\n");
  
  printf("Command line parameters (any is optional):\n");
  printf("-a|b|c|d <filename>\n\tTRD image to be mounted on drive A-D (up to 4 images)\n");
  printf("-com\n\tSerial port name (default = %s)\n", cport);
  printf("-baud\n\tUART Baudrate (default = %d)\n", BAUD);
  printf("-slowpoke\n\tInsert delays into transmit\n");
  printf("-log\n\tScroll log for disk operations\n\n");
}

int parse_arg(int argc, _TCHAR* argv[], _TCHAR* arg, int n)
{
  for (int i=1; i<argc; i++)
    if (!strcmp(argv[i], arg) && (argc-1) >= (i+n))
      return i+1;
  return 0;
}

int parse_args(int argc, _TCHAR* argv[])
{
  int i;

  if (i = parse_arg(argc, argv, "-baud", 1))
    baud = strtol(argv[i], NULL, 10);

  if (i = parse_arg(argc, argv, "-com", 1))
    cport = argv[i];

  if (i = parse_arg(argc, argv, "-log", 0))
    _log = true;

  if (i = parse_arg(argc, argv, "-slowpoke", 0))
    slow = true;

  if (i = parse_arg(argc, argv, "-a", 1))
    { trd[0] = argv[i]; drvs++; }

  if (i = parse_arg(argc, argv, "-b", 1))
    { trd[1] = argv[i]; drvs++; }

  if (i = parse_arg(argc, argv, "-c", 1))
    { trd[2] = argv[i]; drvs++; }

  if (i = parse_arg(argc, argv, "-d", 1))
    { trd[3] = argv[i]; drvs++; }

  return argc > 1;  // drvs
}

U8 update_xor(U8 _xor, U8 *ptr, int num)
{
  while (num--) _xor ^= *ptr++;

  return _xor;
}

//-------------------------------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
  FIFO fifo_in;
  REQ req;
  SECT sect;

  DWORD dwRead, dwWrite;
  STATE state = ST_IDLE;

  printf("\n");

  if (!parse_args(argc, argv))
  {
    print_help();
    return 1;
  }

  for (int i=0; i<4; i++)
  {
    if (!SetDiskImage(i, trd[i], img + i, TRD_SZ))
    {
      // printf("Can't open image %d\n", i);
      return 2;
    }
  }

  HANDLE hPort = OpenSerialPort(cport, baud, 0);

  if (hPort < 0)
  {
    printf("Can't open %s\n", cport);
    return 3;
  }
  else
    printf("%s opened successfully (%d baud)\n\n", cport, baud);

  fifo_init(&fifo_in, fifo_in_buf, sizeof(fifo_in_buf));
  SetInterruptionMask();

  U8 *disk_ptr;
  while (CheckInterruption())
  {
    ReadFile(hPort, uart_in_buf, sizeof(fifo_in_buf), &dwRead, NULL);
    fifo_put(&fifo_in, uart_in_buf, dwRead);

    while (fifo_used(fifo_in))
    {
      switch (state)
      {
        case ST_IDLE:
          if (fifo_get_byte(&fifo_in) == TOKEN1)
            state = ST_RECEIVE_START_TOKEN2;
        break;

        case ST_RECEIVE_START_TOKEN2:
          if (fifo_get_byte(&fifo_in) == TOKEN2)
            state = ST_RECEIVE_HEADER;
          else
            state = ST_IDLE;
        break;

        case ST_RECEIVE_HEADER:
          if (!fifo_get(&fifo_in, (U8*)&req, sizeof(req)))
            goto cont1;
          else
          {
            if (_log) 
              printf("Op: %d\tDrv: %d\tTrk: %d\tSec: %d\n", req.op, req.drv, req.trk, req.sec);
            else
              printf("Op: %d\tDrv: %d\tTrk: %d\tSec: %d \r", req.op, req.drv, req.trk, req.sec);

            if (req.drv > 3)
              if (_log) printf("Wrong drive!\n");

            if (req.sec > 15)
              if (_log) printf("Wrong sector!\n");

            disk_ptr = img[req.drv] + ((req.trk * 16 + req.sec) * sizeof(sect.data));

            switch (req.op)
            {
              case OP_RD:
                sect.ack[0] = ANS1;
                sect.ack[1] = ANS2;
                memcpy(sect.data, disk_ptr, sizeof(sect.data));
                sect.crc = update_xor(ANS1 ^ ANS2, disk_ptr, sizeof(sect.data));
                
                {
                  U8 *ptr = (U8*)&sect;
                  int cnt = sizeof(sect);

                  while (cnt)
                  {
                    int sz = slow ? min(16, cnt) : cnt;
                    WriteFile(hPort, ptr, sz, &dwWrite, NULL);
                    ptr += dwWrite;
                    cnt -= dwWrite;
                    if (slow) Sleep(3);
                  }
                }
                state = ST_IDLE;
              break;

              case OP_WR:
                state = ST_RECEIVE_DATA;
              break;

              default:
                if (_log) printf("Wrong operation!\n");
                state = ST_IDLE;
            }
          }
        break;

        case ST_RECEIVE_DATA:
          if (!fifo_get(&fifo_in, (U8*)&sect, sizeof(sect)))
            goto cont1;
          else
          {
            memcpy(disk_ptr, sect.data, sizeof(sect.data));
            msync(disk_ptr, sizeof(sect.data), MS_ASYNC);
            state = ST_IDLE;
          }
        break;
      }
    }
  cont1: ;
  }

  return 0;
}
