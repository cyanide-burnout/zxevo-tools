TRX2X_FOLDER = ../trx2x

LIBRARIES := \
  stdc++

DIRECTORIES := \
  $(TRX2X_FOLDER)

OBJECTS = \
  $(TRX2X_FOLDER)/DiskImage.o \
  disk.o \
  port.o \
  fifo.o \
  rs232mnt.o

FLAGS := \
  -g -rdynamic -fno-omit-frame-pointer -O2 -MMD -fPIC \
	$(foreach directory, $(DIRECTORIES), -I$(directory))

LIBS := \
  $(foreach library, $(LIBRARIES), -l$(library))

CFLAGS += $(FLAGS) -std=gnu99
CXXFLAGS += $(FLAGS) -fno-implicit-templates -D__STDC_CONSTANT_MACROS

all: build

build: $(PREREQUISITES) $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) $(LIBS) -o rs232mnt

clean:
	rm -f $(PREREQUISITES) $(OBJECTS) rs232mnt
	rm -f $(foreach file, $(OBJECTS), $(subst .o,.d,$(file)))

.PHONY: all build clean install
