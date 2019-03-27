#ifndef _PLAT_RASPI_H
#define _PLAT_RASPI_H

#ifdef __cplusplus
extern "C" {
#endif

// See circle/usb/netdevice.h
#ifndef FRAME_BUFFER_SIZE
#define FRAME_BUFFER_SIZE	1600
#endif

int cr_read(void *pBuffer, unsigned nLength);
int cr_write(const void *pBuffer, unsigned nLength);
int cr_sendframe(const void *pBuffer, unsigned nLength);
// pBuffer must have size FRAME_BUFFER_SIZE
int cr_recvframe(void *pBuffer, unsigned *pResultLength);

void cr_exit(int status) __attribute__ ((noreturn));

#ifdef __cplusplus
}
#endif
#endif
