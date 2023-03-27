
# Makefile for helix test program.
#
#
# $Log: Makefile,v $
# Revision 1.1  2006/05/09 23:49:05  pbrooks
# Added Helix fixed point codec to apd.
#
#

OBJS= main.o debug.o timing.o

LIBS= ../libmp3dec.a

INC=-I../

CC_OPTS= -g -D__GNUC__

CC=gcc

CROSS ?= arm-uclinux-elf

ifdef IPOD
CC = $(CROSS)-gcc
CC_OPTS= -g -DARM 
LD_FLAGS=-elf2flt
endif

all: hmp3_test

hmp3_test: $(OBJS)
	$(CC) $(INC) $(CC_OPTS) -o $@ $(OBJS) $(LIBS) $(LD_FLAGS)

clean:
	rm -rf hmp3_test *.o

main.o: main.c
	$(CC) $(INC)  $(CC_OPTS) -c $< -o $@

timing.o: timing.c
	$(CC) $(INC)  $(CC_OPTS) -c $< -o $@

debug.o: debug.c
	$(CC) $(CC_OPTS) -c $< -o $@
