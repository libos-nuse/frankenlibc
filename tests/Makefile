RUMPOBJ?=../rumpobj
OBJDIR=$(RUMPOBJ)/tests
RUMPDIR?=../rump
LIBDIR=$(RUMPDIR)/lib
BINDIR?=$(RUMPDIR)/bin
REXEC=${BINDIR}/rexec
INCDIR=$(RUMPDIR)/include
LDFLAGS+=-static
CFLAGS+=-g

DEPS=$(LIBDIR)/libc.a

OBJ=$(OBJDIR)/hello $(OBJDIR)/ctor #$(OBJDIR)/hello-pthread
ifneq ($(OS),qemu-arm)
	OBJ+=$(OBJDIR)/cat
endif

default:		$(OBJ)

run-simple:		$(OBJ)
			RUMP_VERBOSE=1 ${REXEC} $(OBJDIR)/hello
			echo frankenlibc | ${REXEC} $(OBJDIR)/cat

run:			$(OBJ)
			dd if=/dev/zero of=disk.img bs=1024 count=20480
			mkfs.ext4 -F disk.img
			RUMP_VERBOSE=1 ${REXEC} $(OBJDIR)/hello disk.img
			${REXEC} $(OBJDIR)/ctor disk.img
ifneq ($(wildcard $(OBJDIR)/cat),)
			echo frankenlibc | ${REXEC} $(OBJDIR)/cat disk.img
endif
ifneq ($(wildcard $(OBJDIR)/ping),)
			${REXEC} ${OBJDIR}/ping disk.img -- -c 3 127.0.0.1
			${REXEC} ${OBJDIR}/ping6 disk.img -- -c 3 ::1
endif
			rm -f disk.img

$(OBJDIR)/ctor:		ctor.c $(DEPS)
			@mkdir -p $(OBJDIR)
			${CC} ${CFLAGS} ${LDFLAGS} -o $@ ctor.c

$(OBJDIR)/hello:	hello.c $(DEPS)
			@mkdir -p $(OBJDIR)
			${CC} ${CFLAGS} ${LDFLAGS} -o $@ hello.c

$(OBJDIR)/hello-pthread: hello-pthread.c $(DEPS)
			@mkdir -p $(OBJDIR)
			${CC} ${CFLAGS} ${LDFLAGS} -o $@ hello-pthread.c
			${CC} ${CFLAGS} -Wl,-rpath=$(LIBDIR)/ -o $@-dl hello-pthread.c

$(OBJDIR)/cat:		cat.c $(DEPS)
			@mkdir -p $(OBJDIR)
			${CC} ${CFLAGS} ${LDFLAGS} -o $@ cat.c

.PHONY:			test clean
clean:		
			rm -f $(OBJ)
			${MAKE} clean -C iputils
