//
// koptions.cpp
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
#include <circle/koptions.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <circle/semihost.h>
#include <circle/sysconfig.h>

#define INVALID_VALUE	((unsigned) -1)

CKernelOptions *CKernelOptions::s_pThis = 0;

CKernelOptions::CKernelOptions (void)
:	m_nWidth (0),
	m_nHeight (0),
	m_nLogLevel (LogDebug),
	m_nUSBPowerDelay (0),
	m_nSoundOption (0),
	m_CPUSpeed (CPUSpeedLow),
	m_nSoCMaxTemp (60)
{
	strcpy (m_LogDevice, "tty1");
	strcpy (m_KeyMap, DEFAULT_KEYMAP);
	m_SoundDevice[0] = '\0';

	s_pThis = this;

	if (!FetchCmdline(&m_TagCommandLine))
	{
		return;
	}

	if (m_TagCommandLine.Tag.nValueLength >= sizeof m_TagCommandLine.String)
	{
		return;
	}
	m_TagCommandLine.String[m_TagCommandLine.Tag.nValueLength] = '\0';

	m_pOptions = (char *) m_TagCommandLine.String;

	char *pOption;
	while ((pOption = GetToken ()) != 0)
	{
		char *pValue = GetOptionValue (pOption);
		ApplyOption (pOption, pValue);
	}

	m_Argc = ParseArgv ();
	m_Envp[m_nEnv] = const_cast<char*>("\0");
}

CKernelOptions::~CKernelOptions (void)
{
	s_pThis = 0;
}

unsigned CKernelOptions::GetWidth (void) const
{
	return m_nWidth;
}

unsigned CKernelOptions::GetHeight (void) const
{
	return m_nHeight;
}

const char *CKernelOptions::GetLogDevice (void) const
{
	return m_LogDevice;
}

unsigned CKernelOptions::GetLogLevel (void) const
{
	return m_nLogLevel;
}

const char *CKernelOptions::GetKeyMap (void) const
{
	return m_KeyMap;
}

unsigned CKernelOptions::GetUSBPowerDelay (void) const
{
	return m_nUSBPowerDelay;
}

const char *CKernelOptions::GetSoundDevice (void) const
{
	return m_SoundDevice;
}

unsigned CKernelOptions::GetSoundOption (void) const
{
	return m_nSoundOption;
}

TCPUSpeed CKernelOptions::GetCPUSpeed (void) const
{
	return m_CPUSpeed;
}

unsigned CKernelOptions::GetSoCMaxTemp (void) const
{
	return m_nSoCMaxTemp;
}

bool CKernelOptions::UseSemihosting (void) const
{
	return m_UseSemihosting;
}

int CKernelOptions::GetArgc (void) const
{
	return m_Argc;
}

const char **CKernelOptions::GetArgv (void) const
{
	return const_cast<const char**>(m_Argv);
}

const char **CKernelOptions::GetEnvp (void) const
{
	return const_cast<const char**>(m_Envp);
}

CKernelOptions *CKernelOptions::Get (void)
{
	return s_pThis;
}

void CKernelOptions::ApplyOption (char *pOption, char *pValue)
{
	if (strcmp (pOption, "width") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && 640 <= nValue && nValue <= 1980)
		{
			m_nWidth = nValue;
		}
	}
	else if (strcmp (pOption, "height") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && 480 <= nValue && nValue <= 1080)
		{
			m_nHeight = nValue;
		}
	}
	else if (strcmp (pOption, "logdev") == 0)
	{
		strncpy (m_LogDevice, pValue, sizeof m_LogDevice-1);
		m_LogDevice[sizeof m_LogDevice-1] = '\0';
	}
	else if (strcmp (pOption, "loglevel") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && nValue <= LogDebug)
		{
			m_nLogLevel = nValue;
		}
	}
	else if (strcmp (pOption, "keymap") == 0)
	{
		strncpy (m_KeyMap, pValue, sizeof m_KeyMap-1);
		m_KeyMap[sizeof m_KeyMap-1] = '\0';
	}
	else if (strcmp (pOption, "usbpowerdelay") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && 200 <= nValue && nValue <= 8000)
		{
			m_nUSBPowerDelay = nValue;
		}
	}
	else if (strcmp (pOption, "sounddev") == 0)
	{
		strncpy (m_SoundDevice, pValue, sizeof m_SoundDevice-1);
		m_SoundDevice[sizeof m_SoundDevice-1] = '\0';
	}
	else if (strcmp (pOption, "soundopt") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && nValue <= 2)
		{
			m_nSoundOption = nValue;
		}
	}
	else if (strcmp (pOption, "fast") == 0)
	{
		if (strcmp (pValue, "true") == 0)
		{
			m_CPUSpeed = CPUSpeedMaximum;
		}
	}
	else if (strcmp (pOption, "socmaxtemp") == 0)
	{
		unsigned nValue;
		if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
		    && 40 <= nValue && nValue <= 78)
		{
			m_nSoCMaxTemp = nValue;
		}
	}
	else if (strcmp (pOption, "env") == 0 && m_nEnv < k_MaxEnvp - 1)
	{
		m_Envp[m_nEnv] = pValue;
		m_nEnv++;
	}
	else if (strcmp (pOption, "semihosting") == 0)
	{
		m_UseSemihosting = (strcmp (pValue, "true") == 0);
	}
}

char *CKernelOptions::GetToken (void)
{
	while (*m_pOptions != '\0')
	{
		if (*m_pOptions != ' ')
		{
			break;
		}

		m_pOptions++;
	}

	if (*m_pOptions == '\0')
	{
		return 0;
	}

	char *pToken = m_pOptions;

	while (*m_pOptions != '\0')
	{
		if (*m_pOptions == ' ')
		{
			*m_pOptions++ = '\0';

			break;
		}

		m_pOptions++;
	}

	if (strcmp (pToken, "--") == 0) {
		return 0;
	}

	return pToken;
}

char *CKernelOptions::GetOptionValue (char *pOption)
{
	while (*pOption != '\0')
	{
		if (*pOption == '=')
		{
			break;
		}

		pOption++;
	}

	if (*pOption == '\0')
	{
		return 0;
	}

	*pOption++ = '\0';

	return pOption;
}

unsigned CKernelOptions::GetDecimal (char *pString)
{
	if (   pString == 0
	    || *pString == '\0')
	{
		return INVALID_VALUE;
	}

	unsigned nResult = 0;

	char chChar;
	while ((chChar = *pString++) != '\0')
	{
		if (!('0' <= chChar && chChar <= '9'))
		{
			return INVALID_VALUE;
		}

		unsigned nPrevResult = nResult;

		nResult = nResult * 10 + (chChar - '0');
		if (   nResult < nPrevResult
		    || nResult == INVALID_VALUE)
		{
			return INVALID_VALUE;
		}
	}

	return nResult;
}

int CKernelOptions::ParseArgv (void)
{
	int argc = 0;

	for (int i = 0; i < k_MaxArgc - 1; i++) {
		auto token = GetArgvToken();
		if (token == nullptr)
		{
			argc = i;
			break;
		}
		m_Argv[i] = token;
		argc++;
	}

	m_Argv[argc] = const_cast<char*>("\0");
	return argc;
}

char *CKernelOptions::GetArgvToken (void)
{
	for ( ; *m_pOptions == ' '; m_pOptions++)
		; // skip leading spaces

	char *pToken = m_pOptions;
	if (*pToken == '\0')
	{
		return nullptr;
	}

	for ( ; *m_pOptions != ' ' && *m_pOptions != '\0'; m_pOptions++)
		; // find the end of the current token

	if (*m_pOptions == ' ')
	{
		*m_pOptions = '\0';
		m_pOptions++;
	}
	return pToken;
}

bool CKernelOptions::FetchCmdline (TPropertyTagCommandLine *pTagCommandLine)
{
	CBcmPropertyTags Tags;
	if (Tags.GetTag (PROPTAG_GET_COMMAND_LINE, pTagCommandLine, sizeof (*pTagCommandLine)))
	{
		return true;
	}

	// We require that semihosting be explicitly turned off via the command
	// line, since we lack a way to reliably detect its support. Since
	// our first attempt resulted in an empty command line, we regard
	// semihosting to have been turned on at this point.
	m_UseSemihosting = true;

	// Retry with semihosting, since QEMU currently doesn't support
	// returning the command line through the mailbox property interface.
	auto nLen = semihost::GetCommandLine (pTagCommandLine->String, pTagCommandLine->kMaxLen);
	if (nLen > 0)
	{
		pTagCommandLine->Tag.nValueLength = nLen;
		return true;
	}

	return false;
}
