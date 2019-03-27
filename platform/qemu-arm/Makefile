COMPILE.c?=$(CC) -c $(CPPFLAGS) $(CFLAGS)
COMPILE.S?=$(CC) -c $(CPPFLAGS) $(ASFLAGS)
INSTALL?=install
RUMPOBJ?=../../rumpobj
OBJDIR=$(RUMPOBJ)/platform
RUMP?=$(RUMPOBJ)/rump
CRTDIR=$(RUMP)/lib
CRTFILES=$(CRTDIR)/startup.o $(CRTDIR)/crt0.o
CRTFILES+=$(CRTDIR)/crti.o $(CRTDIR)/crtn.o
CRTFILES+=$(CRTDIR)/crtbegin.o $(CRTDIR)/crtbeginT.o
CRTFILES+=$(CRTDIR)/crtend.o
CRTFILES+=$(CRTDIR)/link.ld
OBJ=$(CRTFILES)
LIBG=$(CRTDIR)/libg.a
OBJ+=$(LIBG)
OBJ+=$(OBJDIR)/clock_getres.o
OBJ+=$(OBJDIR)/clock_gettime.o
OBJ+=$(OBJDIR)/clock_nanosleep.o
OBJ+=$(OBJDIR)/_exit.o
OBJ+=$(OBJDIR)/fcntl.o
OBJ+=$(OBJDIR)/fstat.o
OBJ+=$(OBJDIR)/fsync.o
OBJ+=$(OBJDIR)/getpagesize.o
OBJ+=$(OBJDIR)/getrandom.o
OBJ+=$(OBJDIR)/kill.o
OBJ+=$(OBJDIR)/lseek.o
OBJ+=$(OBJDIR)/mmap.o
OBJ+=$(OBJDIR)/mprotect.o
OBJ+=$(OBJDIR)/poll.o
OBJ+=$(OBJDIR)/pread.o
OBJ+=$(OBJDIR)/pwrite.o
OBJ+=$(OBJDIR)/preadv.o
OBJ+=$(OBJDIR)/pwritev.o
OBJ+=$(OBJDIR)/read.o
OBJ+=$(OBJDIR)/readv.o
OBJ+=$(OBJDIR)/write.o
OBJ+=$(OBJDIR)/writev.o
OBJ+=$(OBJDIR)/libcircle.o
OBJ+=$(OBJDIR)/libcircleusb.o
OBJ+=$(OBJDIR)/libcircleinput.o
OBJ+=$(OBJDIR)/libcirclefs.o
HEADERS=$(CRTDIR) $(OBJDIR) circle
INCLUDES=-I../include -I../../franken/include -I.
CPPFLAGS+=-nostdinc $(INCLUDES)
CFLAGS+=-fno-stack-protector
RANLIB?=ranlib

default:			$(OBJ)

$(OBJDIR):			
				mkdir -p $(OBJDIR)

$(CRTDIR):			
				mkdir -p $(CRTDIR)

$(CRTDIR)/startup.o:		circle/lib/startup.o $(HEADERS)
				$(INSTALL) $< $@

$(CRTDIR)/crt0.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/crti.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/crtn.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/crtbegin.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/crtbeginT.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/crtend.o:		$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $@ -x c -

$(CRTDIR)/link.ld:		link.ld $(CRTDIR)
				$(INSTALL) -m644 $< $@

$(LIBG):			$(CRTDIR)
				printf "\n" | $(COMPILE.c) -o $(OBJDIR)/g.o -x c -
				$(AR) cr $@ $(OBJDIR)/g.o
				$(RANLIB) $@
				rm -f $(OBJDIR)/g.o
				
$(OBJDIR)/clock_getres.o:	clock_getres.c $(HEADERS)
				$(COMPILE.c) -o $@ clock_getres.c

$(OBJDIR)/clock_gettime.o:	clock_gettime.c $(HEADERS)
				$(COMPILE.c) -o $@ clock_gettime.c

$(OBJDIR)/clock_nanosleep.o:	clock_nanosleep.c $(HEADERS)
				$(COMPILE.c) -o $@ clock_nanosleep.c

$(OBJDIR)/_exit.o:		_exit.c $(HEADERS)
				$(COMPILE.c) -o $@ _exit.c

$(OBJDIR)/fcntl.o:		fcntl.c $(HEADERS)
				$(COMPILE.c) -o $@ fcntl.c

$(OBJDIR)/fstat.o:		fstat.c $(HEADERS)
				$(COMPILE.c) -o $@ fstat.c

$(OBJDIR)/fsync.o:		fsync.c $(HEADERS)
				$(COMPILE.c) -o $@ fsync.c

$(OBJDIR)/getpagesize.o:	getpagesize.c $(HEADERS)
				$(COMPILE.c) -o $@ getpagesize.c

$(OBJDIR)/getrandom.o:		getrandom.c $(HEADERS)
				$(COMPILE.c) -o $@ getrandom.c

$(OBJDIR)/kill.o:		kill.c $(HEADERS)
				$(COMPILE.c) -o $@ kill.c

$(OBJDIR)/lseek.o:		lseek.c $(HEADERS)
				$(COMPILE.c) -o $@ lseek.c

$(OBJDIR)/mmap.o:		mmap.c $(HEADERS)
				$(COMPILE.c) -o $@ mmap.c

$(OBJDIR)/mprotect.o:		mprotect.c
				$(COMPILE.c) -o $@ mprotect.c

$(OBJDIR)/poll.o:		poll.c $(HEADERS)
				$(COMPILE.c) -o $@ poll.c

$(OBJDIR)/pread.o:		pread.c $(HEADERS)
				$(COMPILE.c) -o $@ pread.c

$(OBJDIR)/preadv.o:		preadv.c $(HEADERS)
				$(COMPILE.c) -o $@ preadv.c

$(OBJDIR)/pwrite.o:		pwrite.c $(HEADERS)
				$(COMPILE.c) -o $@ pwrite.c

$(OBJDIR)/pwritev.o:		pwritev.c $(HEADERS)
				$(COMPILE.c) -o $@ pwritev.c

$(OBJDIR)/read.o:		read.c $(HEADERS)
				$(COMPILE.c) -o $@ read.c

$(OBJDIR)/readv.o:		readv.c $(HEADERS)
				$(COMPILE.c) -o $@ readv.c

$(OBJDIR)/write.o:		write.c $(HEADERS)
				$(COMPILE.c) -o $@ write.c

$(OBJDIR)/writev.o:		writev.c $(HEADERS)
				$(COMPILE.c) -o $@ writev.c

$(OBJDIR)/libcircle.o:		circle/lib/libcircle.o $(HEADERS)
				$(INSTALL) $< $@

$(OBJDIR)/libcircleusb.o:	circle/lib/usb/libcircleusb.o $(HEADERS)
				$(INSTALL) $< $@

$(OBJDIR)/libcircleinput.o:	circle/lib/input/libcircleinput.o $(HEADERS)
				$(INSTALL) $< $@

$(OBJDIR)/libcirclefs.o:	circle/lib/fs/libcirclefs.o $(HEADERS)
				$(INSTALL) $< $@

circle: circle/lib/startup.o circle/lib/libcircle.o
circle: circle/lib/usb/libcircleusb.o circle/lib/input/libcircleinput.o
circle: circle/lib/fs/libcirclefs.o

circle/lib/startup.o:
	$(MAKE) -C circle/lib $(notdir $@) $(CIRCLE_FLAGS)

circle/lib/libcircle.o:
	$(MAKE) -C circle/lib $(notdir $@) $(CIRCLE_FLAGS)

circle/lib/usb/libcircleusb.o:
	$(MAKE) -C circle/lib/usb $(notdir $@) $(CIRCLE_FLAGS)

circle/lib/input/libcircleinput.o:
	$(MAKE) -C circle/lib/input $(notdir $@) $(CIRCLE_FLAGS)

circle/lib/fs/libcirclefs.o:
	$(MAKE) -C circle/lib/fs $(notdir $@) $(CIRCLE_FLAGS)

.PHONY:				clean circle
clean:		
				rm -f $(OBJ)
				$(MAKE) -C circle/lib clean
				$(MAKE) -C circle/lib/usb clean
				$(MAKE) -C circle/lib/input clean
				$(MAKE) -C circle/lib/fs clean
