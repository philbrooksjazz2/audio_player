/* $Log: config.h,v $
/* Revision 1.2  2006/05/02 00:54:33  pbrooks
/* Changes to support cross compile to ARM.
/*
/* Revision 1.1  2006/05/01 21:12:55  pbrooks
/* Added config.h and global.h
/* */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to enable diagnostic debugging support. */
#define DEBUG

/* Define to enable experimental code. */
#undef EXPERIMENTAL

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H

/* Define if your MIPS CPU supports a 2-operand MADD16 instruction. */
#undef HAVE_MADD16_ASM

/* Define if your MIPS CPU supports a 2-operand MADD instruction. */
#undef HAVE_MADD_ASM

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H

/* Define to 1 if you have the `pipe' function. */
#define HAVE_PIPE

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* Define to 1 if you have the `waitpid' function. */
#define HAVE_WAITPID

/* Define to disable debugging assertions. */
#undef NDEBUG

/* Define to optimize for accuracy over speed. */
#undef OPT_ACCURACY

/* Define to optimize for speed over accuracy. */
#define OPT_SPEED 1

/* Define to enable a fast subband synthesis approximation optimization. */
#undef OPT_SSO

/* Define to influence a strict interpretation of the ISO/IEC standards, even
   if this is in opposition with best accepted practices. */
#undef OPT_STRICT

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION 1.0

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* fix link error for x86 */
//#define FPM_INTEL 1
#define FPM_ARM 1

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to `int' if <sys/types.h> does not define. */
#undef pid_t
