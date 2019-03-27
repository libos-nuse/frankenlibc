#include <unistd.h>
#include "include/raspi.h"

ssize_t
write(int fd, const void *buf, size_t count)
{
	return cr_write(buf, count);
}
