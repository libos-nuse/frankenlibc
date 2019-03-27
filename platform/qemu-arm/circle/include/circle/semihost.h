//
// semihosting.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2017  R. Stange <rsta2@o2online.de>
// Copyright (C) 2019  midchildan <git@midchildan.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _circle_semihosting_h
#define _circle_semihosting_h

#include <circle/types.h>

namespace semihost {

static constexpr int kSysGetCommandLine = 0x15;
static constexpr int kSysExit = 0x18;

static constexpr u32 kStopBranchThroughZero = 0x20000;
static constexpr u32 kStopUndefinedInstr = 0x20001;
static constexpr u32 kStopSoftwareInterrupt = 0x20002;
static constexpr u32 kStopPrefetchAbort = 0x20003;
static constexpr u32 kStopDataAbort = 0x20004;
static constexpr u32 kStopAddressException = 0x20005;
static constexpr u32 kStopIRQ = 0x20006;
static constexpr u32 kStopFIQ = 0x20007;

static constexpr u32 kStopBreakpoint = 0x20020;
static constexpr u32 kStopWatchpoint = 0x20021;
static constexpr u32 kStopStepComplete = 0x20022;
static constexpr u32 kStopRuntimeErrorUnknown = 0x20023;
static constexpr u32 kStopInternalError = 0x20024;
static constexpr u32 kStopUserInterruption = 0x20025;
static constexpr u32 kStopApplicationExit = 0x20026;
static constexpr u32 kStopStackOverflow = 0x20027;
static constexpr u32 kStopDivisionByZero = 0x20028;
static constexpr u32 kStopOSSpecific = 0x20029;

static inline int Semihost (int nr_syscall, const void *arg)
{
	register int Operation asm("r0") = nr_syscall;
	register const void *Parameter asm("r1") = arg;
	register int Result asm("r0");

	asm volatile ("svc #0x123456"
		      : "=r" (Result)
		      : "0"(Operation), "r"(Parameter)
		      : "r2", "r3", "ip", "lr", "memory", "cc");
		   // Comments from newlib's semihosting code:
		   //
		   // Clobbers r0 and r1, and lr if in supervisor mode
		   //
		   // Accordingly to page 13-77 of ARM DUI 0040D other registers
                   // can also be clobbered.  Some memory positions may also be
                   // changed by a system call, so they should not be kept in
                   // registers. Note: we are assuming the manual is right and
                   // Angel is respecting the APCS.

	// XXX: In ARM's documentation, "Semihosting for AArch32 and AArch64
	// Version 2.0" Ch.3, it is noted that some operations may return
	// additional information in the PARAMETER REGISTER(r1). However, in
	// every case listed, the value of the PARAMETER REGISTER itself is
	// either irrelevant or remains unchanged. Additional information is
	// passed by modifying the data block pointed to by the PARAMETER
	// REGISTER. As far as we're concerned, it should be safe to omit the
	// value of the PARAMETER REGISTER when returning.
	return Result;
}

static inline size_t GetCommandLine (u8* pCommandLine, size_t nMaxLen)
{
	u32 Args[] = {reinterpret_cast<u32>(pCommandLine), nMaxLen};
	auto Err = Semihost(kSysGetCommandLine, Args);
	return (Err == 0) ? Args[1] : 0;
}

static inline int Exit (u32 Reason)
{
	return Semihost(kSysExit, reinterpret_cast<const void*>(Reason));
}

}

#endif
