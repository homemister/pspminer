TARGET = pspminer
OBJS = main.o cencode.o json.o sha256_generic.o

# For OE firmware:
PSP_FW_VERSION = 200
BUILD_PRX = 1


CFLAGS = -O2 -G0 -Wall -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)


LIBDIR =
LDFLAGS =
LIBS = -lpsputility -lpspgum -lpspgu -lm -lpsphttp -lpspssl 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPMINER

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
