#
#
#
#


CC=gcc
CC_OPTS=-g
AMP=./libAMP.a
MAD=../libmad/libmad.a
HELIX=../helix-mp3dec/libmp3dec.a
#LDFLAGS=-lpthread -lm $(AMP)
#LDFLAGS=-lpthread -lm $(MAD) $(HELIX) $(AMP)
LDFLAGS=-lpthread -lm $(HELIX) 



CROSS ?= arm-uclinux-elf

ifdef IPOD
CC = $(CROSS)-gcc
CC_OPTS= -DIPOD -mtune=arm7tdmi -mcpu=arm7tdmi -fomit-frame-pointer -pipe -O3 

#LDFLAGS= $(MAD) $(HELIX) -elf2flt 
LDFLAGS= $(HELIX) -elf2flt 
endif

all: apd

clean:
	rm -rf *.o apd


apd: apd.o apdpcm.o apdutil.o mp3.o 
	$(CC) $(CC_OPTS) -o apd apd.o apdpcm.o apdutil.o mp3.o $(LDFLAGS) 

apd.o: apd.c apd.h
	$(CC) $(CC_OPTS) -c $< -o $@

apdpcm.o: apdpcm.c apdpcm.h
	$(CC) $(CC_OPTS) -c $< -o $@

apdutil.o: apdutil.c apdutil.h
	$(CC) $(CC_OPTS) -c $< -o $@

mp3.o: mp3.c 
	$(CC) $(CC_OPTS) -c $< -o $@
