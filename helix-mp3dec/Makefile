# Makefile for helix fixed point codec.
#
# $Log: Makefile,v $
# Revision 1.1  2006/05/10 00:09:52  pbrooks
# Addd Helix fixed point codec to apd.
#
#
# Add special case for Nokia ARM
LIBNAME= libmp3dec.a
ifneq ($(IPOD),)
#iPod build
#CC= arm-elf-gcc
CC= arm-uclinux-elf-gcc
#AS= arm-elf-as -mfpu=softfpa
AS= arm-uclinux-elf-as -mfpu=softfpa
AR= arm-uclinux-elf-ar
ARM7_OPTS = -mtune=arm7tdmi -mcpu=arm7tdmi -fomit-frame-pointer -pipe  
CFLAGS= $(ARM7_OPTS) -DIPOD -DARM -O3
LDFLAGS= $(LIBNAME) -elf2flt
else
#x86 build
CC= gcc
AS= as
AR= ar
CFLAGS=-DSBOX 
LDFLAGS= $(LIBNAME)
endif

CFLAGS+= -I./pub -I. -Wall -g 

LIBOBJS=\
	mp3dec.o \
	mp3tabs.o
	
ifneq ($(IPP),)
CFLAGS+= -I../ipp/include -DUSE_IPP_MP3 
LIBOBJS+=\
	ipp/bitstream.o \
	ipp/buffers.o \
	ipp/dequant.o \
	ipp/huffman.o \
	ipp/imdct.o \
	ipp/subband.o
else
LIBOBJS+=\
	real/bitstream.o \
	real/buffers.o \
	real/dct32.o \
	real/dequant.o \
	real/dqchan.o \
	real/huffman.o \
	real/hufftabs.o \
	real/imdct.o \
	real/scalfact.o \
	real/stproc.o \
	real/subband.o \
	real/trigtabs.o
# THIS IS A HACK!! - FIX ASAP - add proper defs for scratchbox ARMEL
  ifneq ($(IPOD),)
  LIBOBJS+=\
	real/arm/asmpoly_gcc.o \
	memcpy.o
  else
  LIBOBJS+=\
	real/polyphase.o
  endif
endif

all: mp3dec

mp3dec: $(LIBOBJS)
	$(AR) rc $(LIBNAME) $(LIBOBJS)
clean: 
	$(RM) $(LIBOBJS) $(CLIOBJS) $(LIBNAME) *~
