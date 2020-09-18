#define _DEFAULT_SOURCE

#ifdef __MACH__
#define _DARWIN_C_SOURCE
#endif

#include <termios.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

// Serial port initialization

inline int GetSerialSpeed(int value)
{
  switch (value)
  {
    case 50:       return B50;
    case 75:       return B75;
    case 110:      return B110;
    case 134:      return B134;
    case 150:      return B150;
    case 200:      return B200;
    case 300:      return B300;
    case 600:      return B600;
    case 1200:     return B1200;
    case 1800:     return B1800;
    case 2400:     return B2400;
    case 4800:     return B4800;
    case 9600:     return B9600;
    case 19200:    return B19200;
    case 38400:    return B38400;
    case 57600:    return B57600;
    case 115200:   return B115200;
    case 230400:   return B230400;
#ifdef B460800
    case 460800:   return B460800;
    case 500000:   return B500000;
    case 576000:   return B576000;
    case 921600:   return B921600;
    case 1000000:  return B1000000;
    case 1152000:  return B1152000;
    case 1500000:  return B1500000;
    case 2000000:  return B2000000;
    case 2500000:  return B2500000;
    case 3000000:  return B3000000;
    case 3500000:  return B3500000;
    case 4000000:  return B4000000;
#endif
    default:       return 0;
  }
}

int OpenSerialPort(const char* device, int speed, int flags)
{
  int handle;
  struct termios options;

  handle = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

  if (handle < 0)
  {
    //  Cannot open the port
    return -1;
  }

  if (tcgetattr(handle, &options) < 0)
  {
    close(handle);
    return -1;
  }

  speed  = GetSerialSpeed(speed);
  flags |= CS8 * (flags == 0);

  options.c_cflag  = flags | CREAD | CLOCAL;
  options.c_lflag &= ~ (ECHO | ICANON | IEXTEN | ISIG);

#ifdef __linux__
  options.c_iflag &= ~ (IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IXON | IXOFF | IXANY);
  options.c_oflag &= ~ (OCRNL | ONLCR | ONLRET | ONOCR | OFILL | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY | OLCUC | OPOST);

  options.c_cc[VMIN]  = 1;
  options.c_cc[VTIME] = 0.1;
#endif

#if defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
  options.c_iflag &= ~ (IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IXON | IXOFF | IXANY);
  options.c_oflag &= ~ (OCRNL | ONLCR | ONLRET | ONOCR | OFILL | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY | OPOST);
#endif

  if ((cfsetispeed(&options, speed) < 0) || 
      (cfsetospeed(&options, speed) < 0) ||
      (tcsetattr(handle, TCSANOW, &options) < 0))
  {
    close(handle);
    return -1;
  }

  return handle;
}

// SIGINT handling

void SetInterruptionMask()
{
  sigset_t set;

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL);
}

int CheckInterruption()
{
  sigset_t set;

  return
    ((sigpending(&set) < 0) ||
     (sigismember(&set, SIGINT) == 0));
}

// Win32 API

void Sleep(int interval)
{
  usleep(interval * 1000);
}

void ReadFile(int handle, void* buffer, uint32_t size, uint32_t* count, void* unused)
{
  int result;

  result = read(handle, buffer, size);
  *count = result * (result > 0);
}

void WriteFile(int handle, void* buffer, uint32_t size, uint32_t* count, void* unused)
{
  int result;

  result = write(handle, buffer, size);
  *count = result * (result > 0);
}
