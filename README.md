# POSIX port of TSL tools for ZX Evo

Original tools for Windows - https://github.com/tslabs/zx-evo/tree/master/pentevo/tools

Tested on:
* Linux (Debian 10, Ubuntu 20.04)
* macOS 10.15.6

## rs232mnt

Port of TSL's rs232mnt to POSIX systems with built-in file converter and supports UDI, TRD, SCL, FDI, TD0, FDD formats for read and TRD format for write.

By default, the utility uses the files $TMP/rs232mnt-n.trd (n is a drive number 0..3). All files are mapped to memory by mmap, and any sector will be asynchronously writen to disk.
* When parameters specify the path to a file with the .trd extension (in lowercase), it will be used for reading and writing.
* When a file is empty or created at startup, it will be formatted as TRDOS 80x2.
* When parameters specify a file of any other format (or .TRD in uppercase) - it will be converted to $TMP/rs232mnt-n.trd.

Converter is based on UltraDiskImage engine by Alex Makeev - https://github.com/atsidaev/trx2x/

Topic discussion - http://forum.tslabs.info/viewtopic.php?f=26&t=191&p=31215
