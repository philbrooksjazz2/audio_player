#
#
#
#


DRMLPATH=/home/pmb/src/WMDRMPD_10.06/Source/bind/linux_x11/

DRMLIBS=$(DRMLPATH)cmdlnpars.a $(DRMLPATH)drmmanager.a $(DRMLPATH)drmcore.a $(DRMLPATH)drmdevcert.a $(DRMLPATH)drmdevicedevcert.a $(DRMLPATH)wmdrmcert.a $(DRMLPATH)drmblackbox.a $(DRMLPATH)oem_ansi.a $(DRMLPATH)drmcrypto.a $(DRMLPATH)drmpkcrypto.a $(DRMLPATH)drmbignum.a $(DRMLPATH)drmsynclist.a $(DRMLPATH)drmmetering.a $(DRMLPATH)drmtoolscommon.a

WMA_INC=/home/pmb/src/WMA_codec/WMA9STDDEC/audio/wmaudio/v9/asfparse_s
WMA_INC2=/home/pmb/src/WMA_codec/WMA9STDDEC/audio/wmaudio/v9/common
CC=gcc
CC_OPTS=-g -DDO_WMA -D_LINUX_x86 
#CC_OPTS=-g -DDO_HELIX
#CC_OPTS=-g -DDO_AMP
#CC_OPTS=-g -DDO_MAD
AMP=./libAMP.a
MAD=../libmad/libmad.a
HELIX=../helix-mp3dec/libmp3dec.a
WMA=/home/pmb/src/WMA_codec/WMA9STDDEC/audio/wmaudio/v9/wmadec_s/libwmacodec.a
#LDFLAGS=-lpthread -lm $(AMP)
#LDFLAGS=-lpthread -lm $(MAD) $(AMP)
LDFLAGS=-lpthread -lm $(WMA) $(AMP) $(MAD) $(HELIX) $(DRMLIBS)
#LDFLAGS=-lpthread -lm $(AMP)
#LDFLAGS=-lpthread -lm $(MAD) $(HELIX) $(AMP) 



CROSS ?= arm-uclinux-elf

ifdef IPOD
CC = $(CROSS)-gcc
CC_OPTS= -DIPOD -mtune=arm7tdmi -mcpu=arm7tdmi -fomit-frame-pointer -pipe -O3 

LDFLAGS= $(MAD) $(HELIX) -elf2flt 
endif

all: apd

clean:
	rm -rf *.o apd


apd: apd.o apdpcm.o apdutil.o mp3.o wma.o
	$(CC) $(CC_OPTS) -o apd apd.o apdpcm.o apdutil.o mp3.o wma.o $(LDFLAGS) 

apd.o: apd.c apd.h
	$(CC) $(CC_OPTS) -c $< -o $@

apdpcm.o: apdpcm.c apdpcm.h
	$(CC) $(CC_OPTS) -c $< -o $@

apdutil.o: apdutil.c apdutil.h
	$(CC) $(CC_OPTS) -c $< -o $@

mp3.o: mp3.c 
	$(CC) $(CC_OPTS) -c $< -o $@

wma.o: wma.c 
	$(CC) $(CC_OPTS) -c $< -o $@
