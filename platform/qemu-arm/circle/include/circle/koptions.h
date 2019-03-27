//
// koptions.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2017  R. Stange <rsta2@o2online.de>
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
#ifndef _circle_koptions_h
#define _circle_koptions_h

#include <circle/bcmpropertytags.h>
#include <circle/cputhrottle.h>

class CKernelOptions
{
public:
	CKernelOptions (void);
	~CKernelOptions (void);

	unsigned GetWidth (void) const;
	unsigned GetHeight (void) const;

	const char *GetLogDevice (void) const;
	unsigned GetLogLevel (void) const;

	const char *GetKeyMap (void) const;

	unsigned GetUSBPowerDelay (void) const;

	const char *GetSoundDevice (void) const;	// defaults to empty string
	unsigned GetSoundOption (void) const;

	TCPUSpeed GetCPUSpeed (void) const;
	unsigned GetSoCMaxTemp (void) const;

	bool UseSemihosting (void) const;

	int GetArgc (void) const;
	const char **GetArgv (void) const;
	const char **GetEnvp (void) const;

	static CKernelOptions *Get (void);

private:
	bool FetchCmdline(TPropertyTagCommandLine *pTagCommandLine); // returns true on success, false on failure

	void ApplyOption (char *pOption, char *pValue);
	char *GetToken (void);				// returns next "option=value" pair, 0 if nothing follows

	int ParseArgv (void);
	char *GetArgvToken (void);

	static char *GetOptionValue (char *pOption);	// returns value and terminates option with '\0'

	static unsigned GetDecimal (char *pString);	// returns decimal value, -1 on error

private:
	TPropertyTagCommandLine m_TagCommandLine;
	char *m_pOptions;

	unsigned m_nWidth;
	unsigned m_nHeight;

	char m_LogDevice[20];
	unsigned m_nLogLevel;

	char m_KeyMap[3];

	unsigned m_nUSBPowerDelay;

	char m_SoundDevice[20];
	unsigned m_nSoundOption;

	TCPUSpeed m_CPUSpeed;
	unsigned m_nSoCMaxTemp;

	// Semihosting is initially disabled because it is not safe to use on
	// unsupported platforms. It is eventually turned on by default later in
	// the initialization sequence. We require that semihosting be
	// explicitly turned off via the command line options, since we lack a
	// reliable way to detect its support.
	bool m_UseSemihosting = false;

	static constexpr size_t k_MaxArgc = 20;
	static constexpr size_t k_MaxEnvp = 20;
	int m_Argc = 0;
	char *m_Argv[k_MaxArgc];
	int m_nEnv = 0;
	char *m_Envp[k_MaxEnvp];

	static CKernelOptions *s_pThis;
};

#endif
