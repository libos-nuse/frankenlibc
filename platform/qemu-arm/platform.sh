#!/bin/sh

export CC=${CC-arm-none-eabi-gcc}
export NM=${NM-arm-none-eabi-nm}
export AR=${AR-arm-none-eabi-ar}
export OBJCOPY=${OBJCOPY-arm-none-eabi-objcopy}

# arm-none-eabi-gcc does not turn this off when cpu set
EXTRA_CPPFLAGS="-U__VFP_FP__"

appendvar EXTRA_CFLAGS "-march=armv7-a -marm -mfpu=neon-vfpv4 -mfloat-abi=softfp"
appendvar EXTRA_AFLAGS "-march=armv7-a -marm -mfpu=neon-vfpv4 -D__SOFTFP__"

# this compiler is very fussy, planning to fix these issues at some point
appendvar EXTRA_CWARNFLAGS "-Wno-error"

LINKSCRIPT="${OUTDIR}/lib/link.ld"
EXTRA_LDSCRIPT="-T ${LINKSCRIPT}"
EXTRA_LDSCRIPT_CC="-Wl,-T,${LINKSCRIPT}"

# Still duplicated symbol issues
MAKETOOLS="${MAKETOOLS-no}"
