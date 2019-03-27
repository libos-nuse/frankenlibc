#!/bin/sh

write_log()
{
    if [ -z ${BUILD_QUIET} ] ; then
	if [ "$1" = "-n" ] ; then
	    SECONDS=0
	    /bin/echo -n "=== $1" 1>&1
	    return
	fi
	/bin/echo "$* ($SECONDS sec)" 1>&1
    elif [ "${BUILD_QUIET}" = "-qq" ] ; then
	if [ "$1" = "-n" ] ; then
	    SECONDS=0
	    /bin/echo -n "=== $2" 1>&3
	    return
	fi
	/bin/echo "$* ($SECONDS sec)" 1>&3
    fi

}

MAKE=${MAKE-make}

RUMPOBJ=${PWD}/rumpobj
RUMP=${RUMPOBJ}/rump
RUMPSRC=${PWD}/src
OUTDIR=${PWD}/rump
NCPU=1
RUMP_KERNEL=netbsd

export MAKE
export RUMPSRC
export RUMPOBJ

EXTRA_AFLAGS="-Wa,--noexecstack"

TARGET=$(LC_ALL=C ${CC-cc} -v 2>&1 | sed -n 's/^Target: //p' )

case ${TARGET} in
*-linux*)
	OS=linux
	;;
*-netbsd*)
	OS=netbsd
	;;
*-freebsd*)
	OS=freebsd
	FILTER="-DCAPSICUM"
	;;
*)
	OS=unknown
esac

HOST=$(uname -s)

case ${HOST} in
Linux)
	NCPU=$(nproc 2>/dev/null || echo 1)
	;;
NetBSD)
	NCPU=$(sysctl -n hw.ncpu)
	;;
FreeBSD)
	NCPU=$(sysctl -n hw.ncpu)
	;;
esac

STDJ="-j ${NCPU}"
BUILD_QUIET=""
DBG_F='-O2 -g'

helpme()
{
	printf "Usage: $0 [-h] [options] [platform]\n"
	printf "supported options:\n"
	printf "\t-k: type of rump kernel [netbsd|linux]. default linux\n"
	printf "\t-L: libraries to link eg net_netinet,net_netinet6. default all\n"
	printf "\t-m: hardcode rump memory limit. default from env or unlimited\n"
	printf "\t-M: thread stack size. default: 64k\n"
	printf "\t-p: huge page size to use eg 2M or 1G\n"
	printf "\t-r: release build, without debug settings\n"
	printf "\t-s: location of source tree.  default: PWD/rumpsrc\n"
	printf "\t-o: location of object files. default PWD/rumpobj\n"
	printf "\t-d: location of installed files. default PWD/rump\n"
	printf "\t-b: location of binaries. default PWD/rump/bin\n"
	printf "\tseccomp|noseccomp: select Linux seccomp (default off)\n"
	printf "\texecveat: use new linux execveat call default off)\n"
	printf "\tcapsicum|nocapsicum: select FreeBSD capsicum (default on)\n"
	printf "\tdeterministic: make deterministic\n"
	printf "\tnotests: do not run tests\n"
	printf "\tnotools: do not build extra tools\n"
	printf "\tclean: clean object directory first\n"
	printf "Other options are passed to buildrump.sh\n"
	printf "\n"
	printf "Supported platforms are currently: linux, netbsd, freebsd, qemu-arm, spike\n"
	exit 1
}

bytes()
{
	value=$(echo "$1" | sed 's/[^0123456789].*$//g')
	units=$(echo "$1" | sed 's/^[0123456789]*//g')

	case "$units" in
	"kb"|"k"|"KB"|"K")
		value=$((${value} * 1024))
		;;
	"mb"|"m"|"MB"|"M")
		value=$((${value} * 1048576))
		;;
	"gb"|"g"|"GB"|"G")
		value=$((${value} * 1073741824))
		;;
	*)
		die "Cannot understand value"
		;;
	esac

	echo ${value}
}

abspath() {
    cd "$1"
    printf "$(pwd)"
}

appendvar_fs ()
{
	vname="${1}"
	fs="${2}"
	shift 2
	if [ -z "$(eval echo \${$vname})" ]; then
		eval ${vname}="\${*}"
	else
		eval ${vname}="\"\${${vname}}"\${fs}"\${*}\""
	fi
}

appendvar ()
{

	vname="$1"
	shift
	appendvar_fs "${vname}" ' ' $*
}

. ./buildrump/subr.sh

while getopts '?b:d:F:Hhj:k:L:M:m:o:p:qrs:V:' opt; do
	case "$opt" in
	"b")
		mkdir -p ${OPTARG}
		BINDIR=$(abspath ${OPTARG})
		;;
	"d")
		mkdir -p ${OPTARG}
		OUTDIR=$(abspath ${OPTARG})
		;;
	"F")
		ARG=${OPTARG#*=}
		case ${OPTARG} in
			CFLAGS\=*)
				appendvar EXTRA_CFLAGS "${ARG}"
				;;
			AFLAGS\=*)
				appendvar EXTRA_AFLAGS "${ARG}"
				;;
			LDFLAGS\=*)
				appendvar EXTRA_LDFLAGS "${ARG}"
				;;
			ACFLAGS\=*)
				appendvar EXTRA_CFLAGS "${ARG}"
				appendvar EXTRA_AFLAGS "${ARG}"
				;;
			ACLFLAGS\=*)
				appendvar EXTRA_CFLAGS "${ARG}"
				appendvar EXTRA_AFLAGS "${ARG}"
				appendvar EXTRA_LDFLAGS "${ARG}"
				;;
			CPPFLAGS\=*)
				appendvar EXTRA_CPPFLAGS "${ARG}"
				;;
			DBG\=*)
				appendvar F_DBG "${ARG}"
				;;
			CWARNFLAGS\=*)
				appendvar EXTRA_CWARNFLAGS "${ARG}"
				;;
			*)
				die Unknown flag: ${OPTARG}
				;;
		esac
		;;
	"H")
		appendvar EXTRAFLAGS "-H"
		;;
	"h")
		helpme
		;;
	"j")
		STDJ="-j ${OPTARG}"
		;;
	"k")
		if [ "${OPTARG}" != "netbsd" ] && [ "${OPTARG}" != "linux" ] ; then
			echo ">> ERROR Unknown rump kernel type: ${OPTARG}"
			helpme
		fi
		RUMP_KERNEL="${OPTARG}"
		;;
	"L")
		LIBS="${OPTARG}"
		;;
	"M")
		size=$(bytes ${OPTARG})
		appendvar FRANKEN_FLAGS "-DSTACKSIZE=${size}"
		;;
	"m")
		size=$(bytes ${OPTARG})
		appendvar RUMPUSER_FLAGS "-DRUMP_MEMLIMIT=${size}"
		;;
	"o")
		mkdir -p ${OPTARG}
		RUMPOBJ=$(abspath ${OPTARG})
		RUMP=${RUMPOBJ}/rump
		;;
	"p")
		SIZE=$(bytes ${OPTARG})
		HUGEPAGESIZE="-DHUGEPAGESIZE=${SIZE}"
		;;
	"q")
		BUILD_QUIET=${BUILD_QUIET:=-}q
		;;
	"r")
		DBG_F="-O2"
		appendvar EXTRAFLAGS "-r"
		;;
	"s")
		RUMPSRC=${OPTARG}
		;;
	"V")
		appendvar EXTRAFLAGS "-V ${OPTARG}"
		;;
	"?")
		helpme
		;;
	esac
done
shift $((${OPTIND} - 1))

_cleanup()
{
    cat ${LOG_FILE} 1>&3
    rm -f ${LOG_FILE}
}

if [ -z ${BUILD_QUIET} ] ; then
    set -x
elif [ "${BUILD_QUIET}" = "-qq" ] ; then
    LOG_FILE=$(mktemp)
    exec 3>&1 1>>/dev/null 2>${LOG_FILE}
    trap _cleanup EXIT
fi

if [ -z ${BINDIR+x} ]; then BINDIR=${OUTDIR}/bin; fi

for arg in "$@"; do
        case ${arg} in
	"clean")
		${MAKE} clean
		;;
	"noseccomp")
		;;
	"seccomp")
		appendvar FILTER "-DSECCOMP"
		appendvar TOOLS_LDLIBS "-lseccomp"
		;;
	"noexecveat")
		;;
	"execveat")
		appendvar FILTER "-DEXECVEAT"
		;;
	"nocapsicum")
		FILTER="-DNOCAPSICUM"
		;;
	"capsicum")
		FILTER="-DCAPSICUM"
		;;
	"deterministic"|"det")
		DETERMINISTIC="deterministic"
		;;
	"test"|"tests")
		RUNTESTS="test"
		;;
	"notest"|"notests")
		RUNTESTS="notest"
		;;
	"tools")
		MAKETOOLS="yes"
		;;
	"notools")
		MAKETOOLS="no"
		;;
	*)
		OS=${arg}
		;;
	esac
done

set -e

if [ "${OS}" = "unknown" ]; then
	die "Unknown or unsupported platform"
fi

[ -f platform/${OS}/platform.sh ] && . platform/${OS}/platform.sh
[ -f rumpkernel/${RUMP_KERNEL}.sh ] && . rumpkernel/${RUMP_KERNEL}.sh

RUNTESTS="${RUNTESTS-test}"
MAKETOOLS="${MAKETOOLS-yes}"

rm -rf ${OUTDIR}

FRANKEN_CFLAGS="-std=c99 -Wall -Wextra -Wno-missing-braces -Wno-unused-parameter -Wno-missing-field-initializers"

if [ "${HOST}" = "Linux" ]; then appendvar FRANKEN_CFLAGS "-D_GNU_SOURCE"; fi
if [ "${HOST}" = "FreeBSD" ]; then appendvar FRANKEN_CFLAGS "-D_BSD_SOURCE"; fi

write_log "-n" "building tools.."

CPPFLAGS="${EXTRA_CPPFLAGS} ${FILTER}" \
        CFLAGS="${EXTRA_CFLAGS} ${DBG_F} ${FRANKEN_CFLAGS}" \
        LDFLAGS="${EXTRA_LDFLAGS}" \
        LDLIBS="${TOOLS_LDLIBS}" \
        RUMPOBJ="${RUMPOBJ}" \
        RUMP="${RUMP}" \
        ${MAKE} ${OS} -C tools
write_log " done"

# call buildrump.sh
write_log "-n" "building rumpkernel.."
rumpkernel_buildrump
write_log " done"

# remove libraries that are not/will not work
rm -f ${RUMP}/lib/librumpdev_ugenhc.a
rm -f ${RUMP}/lib/librumpfs_syspuffs.a
rm -f ${RUMP}/lib/librumpkern_sysproxy.a
rm -f ${RUMP}/lib/librumpnet_shmif.a
rm -f ${RUMP}/lib/librumpnet_sockin.a
rm -f ${RUMP}/lib/librumpvfs_fifofs.a
rm -f ${RUMP}/lib/librumpdev_netsmb.a
rm -f ${RUMP}/lib/librumpfs_smbfs.a
rm -f ${RUMP}/lib/librumpdev_usb.a
rm -f ${RUMP}/lib/librumpdev_ucom.a
rm -f ${RUMP}/lib/librumpdev_ulpt.a
rm -f ${RUMP}/lib/librumpdev_ubt.a
rm -f ${RUMP}/lib/librumpkern_sys_linux.a
rm -f ${RUMP}/lib/librumpdev_umass.a

# remove crypto for now as very verbose
rm -f ${RUMP}/lib/librumpkern_crypto.a
rm -f ${RUMP}/lib/librumpdev_opencrypto.a
rm -f ${RUMP}/lib/librumpdev_cgd.a

# userspace libraries to build from NetBSD base
USER_LIBS="m pthread z crypt util prop rmt ipsec"
NETBSDLIBS="${RUMPSRC}/lib/libc"
for f in ${USER_LIBS}
do
        appendvar NETBSDLIBS "${RUMPSRC}/lib/lib${f}"
done

RUMPMAKE=${RUMPOBJ}/tooldir/rumpmake

# build userspace library for rumpkernel
write_log "-n" "building userspace libs.."
rumpkernel_createuserlib
write_log " done"

write_log "-n" "building userspace headers.."

# permissions set wrong
chmod -R ug+rw ${RUMP}/include/*
# install headers
${INSTALL-install} -d ${OUTDIR}/include

# install headers of userspace lib
rumpkernel_install_header
write_log " done"

write_log "-n" "building platform libs.."

CFLAGS="${EXTRA_CFLAGS} ${DBG_F} ${HUGEPAGESIZE} ${FRANKEN_CFLAGS}" \
	AFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	ASFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	LDFLAGS="${EXTRA_LDFLAGS}" \
	CPPFLAGS="${EXTRA_CPPFLAGS}" \
	RUMPOBJ="${RUMPOBJ}" \
	RUMP="${RUMP}" \
	${MAKE} ${STDJ} ${OS} -C platform

# should clean up how deterministic build is done
if [ ${DETERMINSTIC-x} = "deterministic" ]
then
	CFLAGS="${EXTRA_CFLAGS} ${DBG_F} ${HUGEPAGESIZE} ${FRANKEN_CFLAGS}" \
	AFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	ASFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	LDFLAGS="${EXTRA_LDFLAGS}" \
	CPPFLAGS="${EXTRA_CPPFLAGS}" \
	RUMPOBJ="${RUMPOBJ}" \
	RUMP="${RUMP}" \
	${MAKE} deterministic -C platform
fi 

write_log " done"
write_log "-n" "building franken libs.."

CFLAGS="${EXTRA_CFLAGS} ${DBG_F} ${HUGEPAGESIZE} ${FRANKEN_CFLAGS}" \
	AFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	ASFLAGS="${EXTRA_AFLAGS} ${DBG_F}" \
	LDFLAGS="${EXTRA_LDFLAGS}" \
	CPPFLAGS="${EXTRA_CPPFLAGS} ${FRANKEN_FLAGS}" \
	RUMPOBJ="${RUMPOBJ}" \
	RUMP="${RUMP}" \
	${MAKE} ${STDJ} -C franken

write_log " done"
write_log "-n" "building rumpuser libs.."

CFLAGS="${EXTRA_CFLAGS} ${DBG_F} ${FRANKEN_CFLAGS}" \
	LDFLAGS="${EXTRA_LDFLAGS}" \
	CPPFLAGS="${EXTRA_CPPFLAGS} ${RUMPUSER_FLAGS}" \
	RUMPOBJ="${RUMPOBJ}" \
	RUMP="${RUMP}" \
	${MAKE} ${STDJ} -C librumpuser

write_log " done"

# build extra library
write_log "-n" "building extra files.."
rumpkernel_build_extra
write_log "done"

write_log "-n" "building special libc for cross build.."

# find which libs we should link
ALL_LIBS="${RUMP}/lib/librump.a
	${RUMP}/lib/libfranken_tc.a
	${RUMP}/lib/librumpdev*.a
	${RUMP}/lib/librumpnet*.a
	${RUMP}/lib/librumpfs*.a
	${RUMP}/lib/librumpvfs*.a
	${RUMP}/lib/librumpkern*.a"

if [ ! -z ${LIBS+x} ]
then
	ALL_LIBS="${RUMP}/lib/librump.a ${RUMP}/lib/libfranken_tc.a"
	for l in $(echo ${LIBS} | tr "," " ")
	do
		case ${l} in
		dev_*)
			appendvar ALL_LIBS "${RUMP}/lib/librumpdev.a"
			;;
		net_*)
			appendvar ALL_LIBS "${RUMP}/lib/librumpnet.a ${RUMP}/lib/librumpnet_net.a"
			;;
		vfs_*)
			appendvar ALL_LIBS "${RUMP}/lib/librumpvfs.a"
			;;
		fs_*)
			appendvar ALL_LIBS "${RUMP}/lib/librumpvfs.a ${RUMP}/lib/librumpdev.a ${RUMP}/lib/librumpdev_disk.a"
			;;
		esac
		appendvar ALL_LIBS "${RUMP}/lib/librump${l}.a"
	done
fi

# for Linux case
if [ ${RUMP_KERNEL} != "netbsd" ]
then
	ALL_LIBS=${RUMPOBJ}/linux/tools/lkl/liblkl.a
fi

# explode and implode
rm -rf ${RUMPOBJ}/explode
mkdir -p ${RUMPOBJ}/explode/libc
mkdir -p ${RUMPOBJ}/explode/musl
mkdir -p ${RUMPOBJ}/explode/rumpkernel
mkdir -p ${RUMPOBJ}/explode/rumpuser
mkdir -p ${RUMPOBJ}/explode/franken
mkdir -p ${RUMPOBJ}/explode/platform
(
	# explode rumpkernel specific libc
	rumpkernel_explode_libc

	# some franken .o file names conflict with libc
	cd ${RUMPOBJ}/explode/franken
	${AR-ar} x ${RUMP}/lib/libfranken.a
	for f in *.o
	do
		[ -f ../${LIBC_DIR}/$f ] && mv $f franken_$f
	done

	# some platform .o file names conflict with libc
	cd ${RUMPOBJ}/explode/platform
	${AR-ar} x ${RUMP}/lib/libplatform.a
	for f in *.o
	do
		[ -f ../libc/$f ] && mv $f platform_$f
	done

	cd ${RUMPOBJ}/explode/rumpkernel
	for f in ${ALL_LIBS}
	do
		${AR-ar} x $f
	done
	${CC-cc} ${EXTRA_LDFLAGS} -nostdlib -Wl,-r *.o -o rumpkernel.o

	cd ${RUMPOBJ}/explode/rumpuser
	${AR-ar} x ${RUMP}/lib/librumpuser.a

	cd ${RUMPOBJ}/explode
	${AR-ar} cr libc.a rumpkernel/rumpkernel.o rumpuser/*.o ${LIBC_DIR}/*.o franken/*.o platform/*.o
)

write_log " done"
write_log "-n" "installing from staging directory.."

# install to OUTDIR
${INSTALL-install} -d ${BINDIR} ${OUTDIR}/lib
${INSTALL-install} ${RUMP}/bin/rexec ${BINDIR}

# call rumpkernel specific routine
rumpkernel_install_extra_libs

${INSTALL-install} ${RUMP}/lib/*.o ${OUTDIR}/lib
[ -f ${RUMP}/lib/link.ld ] && ${INSTALL-install} ${RUMP}/lib/link.ld ${OUTDIR}/lib
[ -f ${RUMP}/lib/libg.a ] && ${INSTALL-install} ${RUMP}/lib/libg.a ${OUTDIR}/lib
${INSTALL-install} ${RUMPOBJ}/explode/libc.a ${OUTDIR}/lib

# create toolchain wrappers
# select these based on compiler defs
if $(${CC-cc} -v 2>&1 | grep -q clang)
then
	TOOL_PREFIX=$(basename $(ls ${RUMPOBJ}/tooldir/bin/*-clang) | sed -e 's/-clang//' -e "s/--netbsd/-rumprun-${RUMP_KERNEL}/")
	# possibly some will need to be filtered if compiler complains. Also de-dupe.
	COMPILER_FLAGS="-fno-stack-protector -Wno-unused-command-line-argument ${EXTRA_CPPFLAGS} ${UNDEF} ${EXTRA_CFLAGS} ${EXTRA_LDSCRIPT_CC}"
	COMPILER_FLAGS="$(echo ${COMPILER_FLAGS} | sed 's/--sysroot=[^ ]*//g')"
	# set up sysroot to see if it works
	( cd ${OUTDIR} && ln -s . usr )
	LIBGCC="$(${CC-cc} ${EXTRA_CPPFLAGS} ${EXTRA_CFLAGS} -print-libgcc-file-name)"
	LIBGCCDIR="$(dirname ${LIBGCC})"
	ln -s ${LIBGCC} ${OUTDIR}/lib/
	ln -s ${LIBGCCDIR}/libgcc_eh.a ${OUTDIR}/lib/
	if ${CC-cc} -I${OUTDIR}/include --sysroot=${OUTDIR} -static ${COMPILER_FLAGS} tests/hello.c -o /dev/null 2>/dev/null
	then
		# can use sysroot with clang
		printf "#!/bin/sh\n\nexec ${CC-cc} --sysroot=${OUTDIR} -static ${COMPILER_FLAGS} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-clang
	else
		# sysroot does not work with linker eg NetBSD
		appendvar COMPILER_FLAGS "-I${OUTDIR}/include -L${OUTDIR}/lib -B${OUTDIR}/lib"
		printf "#!/bin/sh\n\nexec ${CC-cc} -static ${COMPILER_FLAGS} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-clang
	fi
	COMPILER="${TOOL_PREFIX}-clang"
	( cd ${BINDIR}
	  ln -s ${COMPILER} ${TOOL_PREFIX}-cc
	  ln -s ${COMPILER} rumprun-cc
	)
else
	# spec file for gcc
	TOOL_PREFIX=$(basename $(ls ${RUMPOBJ}/tooldir/bin/*-gcc) | \
			  sed -e 's/-gcc//' -e "s/--netbsd/-rumprun-${RUMP_KERNEL}/")
	COMPILER_FLAGS="-fno-stack-protector ${EXTRA_CFLAGS}"
	COMPILER_FLAGS="$(echo ${COMPILER_FLAGS} | sed 's/--sysroot=[^ ]*//g')"
	[ -f ${OUTDIR}/lib/crt0.o ] && appendvar STARTFILE "${OUTDIR}/lib/crt0.o"
	[ -f ${OUTDIR}/lib/crt1.o ] && appendvar STARTFILE "${OUTDIR}/lib/crt1.o"
	appendvar STARTFILE "${OUTDIR}/lib/crti.o"
	[ -f ${OUTDIR}/lib/crtbegin.o ] && appendvar STARTFILE "${OUTDIR}/lib/crtbegin.o"
	[ -f ${OUTDIR}/lib/crtbeginT.o ] && appendvar STARTFILE "${OUTDIR}/lib/crtbeginT.o"
	ENDFILE="${OUTDIR}/lib/crtend.o ${OUTDIR}/lib/crtn.o"
	cat tools/spec.in | sed \
		-e "s#@SYSROOT@#${OUTDIR}#g" \
		-e "s#@CPPFLAGS@#${EXTRA_CPPFLAGS}#g" \
		-e "s#@AFLAGS@#${EXTRA_AFLAGS}#g" \
		-e "s#@CFLAGS@#${EXTRA_CFLAGS}#g" \
		-e "s#@LDFLAGS@#${EXTRA_LDFLAGS}#g" \
		-e "s#@LDSCRIPT@#${EXTRA_LDSCRIPT}#g" \
		-e "s#@UNDEF@#${UNDEF}#g" \
		-e "s#@STARTFILE@#${STARTFILE}#g" \
		-e "s#@ENDFILE@#${ENDFILE}#g" \
		-e "s/--sysroot=[^ ]*//" \
		> ${OUTDIR}/lib/${TOOL_PREFIX}gcc.spec
	printf "#!/bin/sh\n\nexec ${CC-cc} -specs ${OUTDIR}/lib/${TOOL_PREFIX}gcc.spec ${COMPILER_FLAGS} -static -nostdinc -isystem ${OUTDIR}/include \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-gcc
	COMPILER="${TOOL_PREFIX}-gcc"
	( cd ${BINDIR}
	  ln -s ${COMPILER} ${TOOL_PREFIX}-cc
	  ln -s ${COMPILER} rumprun-cc
	)
fi
printf "#!/bin/sh\n\nexec ${AR-ar} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-ar
printf "#!/bin/sh\n\nexec ${NM-nm} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-nm
printf "#!/bin/sh\n\nexec ${LD-ld} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-ld
printf "#!/bin/sh\n\nexec ${OBJCOPY-objcopy} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-objcopy
printf "#!/bin/sh\n\nexec ${OBJDUMP-objdump} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-objdump
printf "#!/bin/sh\n\nexec ${RANLIB-ranlib} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-ranlib
printf "#!/bin/sh\n\nexec ${READELF-readelf} \"\$@\"\n" > ${BINDIR}/${TOOL_PREFIX}-readelf
chmod +x ${BINDIR}/${TOOL_PREFIX}-*

write_log " done"
write_log "-n" "testing duplicated syms.."

# test for duplicated symbols

DUPSYMS=$(nm ${OUTDIR}/lib/libc.a | grep ' T ' | sed 's/.* T //g' | sort | uniq -d )

if [ -n "${DUPSYMS}" ]
then
	printf "WARNING: Duplicate symbols found:\n"
	echo ${DUPSYMS}
	#exit 1
fi

# install some useful applications
if [ ${MAKETOOLS} = "yes" ]
then
	rumpkernel_maketools
fi

write_log " done"
write_log "-n" "building tests.."

# Always make tests to exercise compiler
CC="${BINDIR}/${COMPILER}" \
	RUMPDIR="${OUTDIR}" \
	RUMPOBJ="${RUMPOBJ}" \
	BINDIR="${BINDIR}" \
	OS="${OS}" \
	${MAKE} ${STDJ} -C tests

# test for executable stack
readelf -lW ${RUMPOBJ}/tests/hello | grep RWE 1>&2 && echo "WARNING: writeable executable section (stack?) found" 1>&2

rumpkernel_build_test

write_log " done"
write_log "-n" "running tests.."

if [ ${RUNTESTS} = "test" ]
then
	CC="${BINDIR}/${COMPILER}" \
		RUMPDIR="${OUTDIR}" \
		RUMPOBJ="${RUMPOBJ}" \
		BINDIR="${BINDIR}" \
		${MAKE} -C tests run

fi

write_log " done"
