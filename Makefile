
TARGET = libcddaex.so

CFLAGS += -I$(SDKSTAGE)/usr/include/
CFLAGS += -I$(SDKSTAGE)/usr/include/iopccommon

LDFLAGS += -L$(SDKSTAGE)/usr/lib/
LDFLAGS += -liopccommon

SRC = 
SRC += ops_cddaex.c

include Makefile.include.lib
