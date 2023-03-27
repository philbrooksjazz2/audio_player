#
# Makefile for libmad - mp3 audio decoder.
#
# $Log: Makefile,v $
# Revision 1.5  2006/05/04 23:32:58  pbrooks
# Added suppor for ASO_IMDCT - assembly routines for ARM.
#
# Revision 1.4  2006/05/04 17:39:41  pbrooks
# Added libmad.a to clean.
#
# Revision 1.3  2006/05/02 17:07:22  pbrooks
# Added suport to build libmad.a
#
# Revision 1.2  2006/05/02 00:54:33  pbrooks
# Changes to support cross compile to ARM.
#
# Revision 1.1  2006/05/01 18:03:52  pbrooks
# Initial checkin of mad mp3 decoder.
#
#


LIBMAD=libmad.a

#CC_OPTS= -g -DFPM_INTEL
CC_OPTS= -g -DFPM_ARM -DASO_IMDCT
LDFLAGS=-elf2flt
CC=gcc
AR=ar
AR_OPTS=-crs
srcdir=.

CROSS ?= arm-uclinux-elf

ifdef IPOD
CC = $(CROSS)-gcc

CC_OPTS= -g -DFPM_ARM -DASO_IMDCT
LDFLAGS=-elf2flt
endif


OBJS=bit.o \
     decoder.o \
     fixed.o \
     frame.o \
     huffman.o \
     layer12.o \
     layer3.o \
     stream.o \
     synth.o \
     timer.o \
     version.o

ARM_OBJS=imdct_l_arm.o
     
ifdef IPOD
all: $(OBJS) $(LIBMAD) $(ARM_OBJS)
else
all: $(OBJS) $(LIBMAD)
endif

test: minimad

minimad: minimad.o $(OBJS)
	$(CC) -o $@ $< $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o minimad $(LIBMAD)

ifdef IPOD
$(LIBMAD): $(OBJS) $(ARM_OBJS)
	$(AR) $(AR_OPTS) $(LIBMAD) $(OBJS) $(ARM_OBJS) 
else
$(LIBMAD): $(OBJS)
	$(AR) $(AR_OPTS) $(LIBMAD) $(OBJS) 
endif

bit.o: bit.c 
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
decoder.o: decoder.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
fixed.o: fixed.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
frame.o: frame.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
huffman.o: huffman.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
layer12.o: layer12.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
layer3.o: layer3.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
minimad.o: minimad.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
stream.o: stream.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
synth.o: synth.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
timer.o: timer.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
version.o: version.c
	$(CC) $(INC) $(CC_OPTS) -c $< -o $@
imdct_l_arm.o: imdct_l_arm.S
	$(CC) $(CC_OPTS) -c imdct_l_arm.S -o $@

mad.h: config.h Makefile.am  \
		$(srcdir)/COPYRIGHT $(srcdir)/mad.h.sed $(exported_headers)
	(sed -e '1s|.*|/*|' -e '1b' -e '$$s|.*| */|' -e '$$b'  \
		-e 's/^.*/ *&/' $(srcdir)/COPYRIGHT; echo;  \
	echo "# ifdef __cplusplus";  \
	echo 'extern "C" {';  \
	echo "# endif"; echo;  \
	if [ ".$(FPM)" != "." ]; then  \
		echo ".$(FPM)" | sed -e 's|^\.-D|# define |'; echo;  \
	fi;  \
	sed -ne 's/^# *define  *\(HAVE_.*_ASM\).*/# define \1/p'  \
		config.h; echo;  \
	sed -ne 's/^# *define  *OPT_\(SPEED\|ACCURACY\).*/# define OPT_\1/p'  \
		config.h; echo;  \
	sed -ne 's/^# *define  *\(SIZEOF_.*\)/# define \1/p'  \
		config.h; echo;  \
	for header in $(exported_headers); do  \
		echo;  \
		sed -n -f $(srcdir)/mad.h.sed $(srcdir)/$$header;  \
	done; echo;  \
	echo "# ifdef __cplusplus";  \
	echo '}';  \
	echo "# endif") >$@



