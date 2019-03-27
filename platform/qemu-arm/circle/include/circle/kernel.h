#ifndef __circle_kernel
#define __circle_kernel

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/dwhcidevice.h>
#include <circle/usb/netdevice.h>
#include <circle/types.h>

extern "C" {
using mainfn_t = int(*)(int, char**, char**);
extern int __franken_start_main(mainfn_t, int, const char **, const char **);
extern int main(int, char**, char**);
}

void cr_main(void);

class CKernel {
    public:
	CKernel(void);
	~CKernel(void) = default;

	boolean Initialize(void);

	inline int GetArgc(void) const
	{
		return m_Options.GetArgc();
	}
	inline const char **GetArgv(void) const
	{
		return m_Options.GetArgv();
	}
	inline const char **GetEnvp(void) const
	{
		return m_Options.GetEnvp();
	}

	inline boolean UseSemihosting (void) const
	{
		return m_Options.UseSemihosting();
	}

	inline boolean ReceiveFrame(void *pBuffer, unsigned *pResultLength)
	{
		return m_Eth0->ReceiveFrame(pBuffer, pResultLength);
	}

	inline boolean SendFrame(const void *pBuffer, unsigned nLength)
	{
		return m_Eth0->SendFrame(pBuffer, nLength);
	}

	inline int Read(void *pBuffer, unsigned nLength) {
		return m_logDst->Write(pBuffer, nLength);
	}

	inline int Write(const void *pBuffer, unsigned nLength) {
		return m_logDst->Write(pBuffer, nLength);
	}

    private:
	// do not change this order
	CMemorySystem m_Memory;
	CActLED m_ActLED;
	CKernelOptions m_Options;
	CDeviceNameService m_DeviceNameService;
	CScreenDevice m_Screen;
	CSerialDevice m_Serial;
	CExceptionHandler m_ExceptionHandler;
	CInterruptSystem m_Interrupt;
	CTimer m_Timer;
	CLogger m_Logger;
	CDWHCIDevice m_DWHCI;

	CDevice* m_logDst = nullptr;
	CNetDevice* m_Eth0 = nullptr;
};

#endif
