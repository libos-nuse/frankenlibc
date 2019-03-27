#include <circle/kernel.h>
#include <circle/semihost.h>
#include <circle/startup.h>
#include "../../include/raspi.h"

#define CKERNEL_INIT(dev, ...)                                                 \
	if (!(dev).Initialize(__VA_ARGS__)) {                                  \
		return FALSE;                                                  \
	}

static CKernel *s_kernel = nullptr;

CKernel::CKernel(void)
	: m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
	  m_Timer(&m_Interrupt), m_Logger(m_Options.GetLogLevel(), &m_Timer),
	  m_DWHCI(&m_Interrupt, &m_Timer)
{
	m_ActLED.Blink(5); // show we are alive
}

boolean CKernel::Initialize(void)
{
	CKERNEL_INIT(m_Screen);
	CKERNEL_INIT(m_Serial, 115200);

	const char* logdevice = m_Options.GetLogDevice();
	m_logDst = m_DeviceNameService.GetDevice(logdevice, FALSE);
	if (m_logDst == nullptr) {
		m_logDst = &m_Screen;
	}

	CKERNEL_INIT(m_Logger, m_logDst);
	CKERNEL_INIT(m_Interrupt);
	CKERNEL_INIT(m_Timer);
	CKERNEL_INIT(m_DWHCI);

	m_Eth0 = (CNetDevice *)m_DeviceNameService.GetDevice("eth0", FALSE);
	if (m_Eth0 == nullptr) {
		m_Logger.Write("CKernel", LogError, "Net device not found");
		return FALSE;
	}

	return TRUE;
}

size_t __hwcap = 407;

extern "C" int cr_read(void *pBuffer, unsigned nLength)
{
	return s_kernel ? s_kernel->Read(pBuffer, nLength) : 0;
}

extern "C" int cr_write(const void *pBuffer, unsigned nLength)
{
	return s_kernel ? s_kernel->Write(pBuffer, nLength) : 0;
}

extern "C" int cr_sendframe(const void *pBuffer, unsigned nLength)
{
	return s_kernel ? s_kernel->SendFrame(pBuffer, nLength) : 0;
}

// pBuffer must have size FRAME_BUFFER_SIZE
extern "C" int cr_recvframe(void *pBuffer, unsigned *pResultLength)
{
	return s_kernel ? s_kernel->ReceiveFrame(pBuffer, pResultLength) : 0;
}

extern "C" void cr_exit(int status)
{
	if (!s_kernel) {
		goto fallback;
	}

	if (s_kernel->UseSemihosting()) {
		auto reason = semihost::kStopApplicationExit;
		if (status != 0) {
			reason = semihost::kStopInternalError;
		}
		semihost::Exit(reason);
	}

fallback:
	s_kernel = nullptr;
	halt();
}

void cr_main(void)
{
	CKernel kernel;
	s_kernel = &kernel;
	int exitCode = 0;

        int argc = kernel.GetArgc();
        const char **argv = kernel.GetArgv();
        const char **envp = kernel.GetEnvp();

	if (!kernel.Initialize()) {
		exitCode = -1;
		goto exit;
	}

	exitCode = __franken_start_main(main, argc, argv, envp);

exit:
	cr_exit(exitCode);
}
