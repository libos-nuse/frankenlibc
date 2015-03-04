/*	$NetBSD: rumpuser_port.h,v 1.42 2014/11/10 22:43:46 pooka Exp $	*/

#ifndef _LIB_LIBRUMPUSER_RUMPUSER_PORT_H_
#define _LIB_LIBRUMPUSER_RUMPUSER_PORT_H_

#define HAVE_ALIGNED_ALLOC 1
#define HAVE_CLOCKID_T 1
#define HAVE_CLOCK_GETTIME 1
#define HAVE_CLOCK_NANOSLEEP 1
#define HAVE_GETRANDOM 1
#define HAVE_INTTYPES_H 1
#define HAVE_POSIX_MEMALIGN 1
#define HAVE_REGISTER_T 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_CDEFS_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define STDC_HEADERS 1

#include <sys/cdefs.h>

#ifndef MIN
#define MIN(a,b)        ((/*CONSTCOND*/(a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)        ((/*CONSTCOND*/(a)>(b))?(a):(b))
#endif

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

#ifndef _DIAGASSERT
#define _DIAGASSERT(_p_)
#endif

#ifndef __predict_true
#define __predict_true(a) a
#define __predict_false(a) a
#endif

#ifndef __printflike
#ifdef __GNUC__
#define __printflike(a,b) __attribute__((__format__ (__printf__,a,b)))
#else
#define __printflike(a,b)
#endif
#endif

#ifndef __noinline
#ifdef __GNUC__
#define __noinline __attribute__((__noinline__))
#else
#define __noinline
#endif
#endif

#ifndef __arraycount
#define __arraycount(_ar_) (sizeof(_ar_)/sizeof(_ar_[0]))
#endif

#ifndef __UNCONST
#define __UNCONST(_a_) ((void *)(unsigned long)(const void *)(_a_))
#endif

#ifndef __CONCAT
#define __CONCAT(x,y)	x ## y
#endif

#ifndef __STRING
#define __STRING(x)	#x
#endif

#ifndef __NetBSD_Prereq__
#define __NetBSD_Prereq__(a,b,c) 0
#endif

#if !defined(HAVE_REGISTER_T) && !defined(RUMP_REGISTER_T)
#define RUMP_REGISTER_T long
typedef RUMP_REGISTER_T register_t;
#endif

#include <sys/time.h>

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts)		\
do {						\
	(ts)->tv_sec  = (tv)->tv_sec;		\
	(ts)->tv_nsec = (tv)->tv_usec * 1000;	\
} while (/*CONSTCOND*/0)
#endif

#endif /* _LIB_LIBRUMPUSER_RUMPUSER_PORT_H_ */
