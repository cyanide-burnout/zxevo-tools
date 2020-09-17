OBJECTS = \
  port.o \
  fifo.o \
  rs232mnt.o

FLAGS := \
  -g -rdynamic -fno-omit-frame-pointer -O2 -MMD -fPIC

LIBS := \
  $(foreach library, $(LIBRARIES), -l$(library))

CFLAGS += $(FLAGS) -std=gnu99
CXXFLAGS += $(FLAGS) -fno-implicit-templates -D__STDC_CONSTANT_MACROS

all: build

build: $(PREREQUISITES) $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) $(LIBS) -o rs232mnt

clean:
	rm -f $(PREREQUISITES) $(OBJECTS) rs232mnt
	rm *.d

.PHONY: all build clean install
