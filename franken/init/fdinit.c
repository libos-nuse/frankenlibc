#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "init.h"

static int disk_id;
union lkl_disk {
	int fd;
	void *handle;
};
long lkl_mount_dev(unsigned int disk_id, const char *fs_type, int flags,
		   void *data, char *mnt_str, unsigned int mnt_str_len);
int lkl_disk_add(union lkl_disk disk);
long lkl_umount_dev(char *mnt_str, int flags);

static int nd_id;
union lkl_netdev {
        int fd;
};
int lkl_netdev_add(union lkl_netdev nd, void *mac);
int lkl_if_up(int ifindex);
int lkl_netdev_get_ifindex(int id);

typedef unsigned short      umode_t;

long lkl_syscall(long no, long *params);
#define __LKL__MAP0(m,...)
#define __LKL__MAP1(m,t,a) m(t,a)
#define __LKL__MAP2(m,t,a,...) m(t,a), __LKL__MAP1(m,__VA_ARGS__)
#define __LKL__MAP3(m,t,a,...) m(t,a), __LKL__MAP2(m,__VA_ARGS__)
#define __LKL__MAP4(m,t,a,...) m(t,a), __LKL__MAP3(m,__VA_ARGS__)
#define __LKL__MAP5(m,t,a,...) m(t,a), __LKL__MAP4(m,__VA_ARGS__)
#define __LKL__MAP6(m,t,a,...) m(t,a), __LKL__MAP5(m,__VA_ARGS__)
#define __LKL__MAP(n,...) __LKL__MAP##n(__VA_ARGS__)

#define __LKL__SC_LONG(t, a) (long)a
#define __LKL__SC_DECL(t, a) t a

#define LKL_SYSCALL0(name)                         \
    static __inline__ long lkl_sys##name(void)                 \
    {                                  \
        long params[6];                        \
        return lkl_syscall(__lkl__NR##name, params);           \
    }

#define LKL_SYSCALLx(x, name, ...)                     \
        static __inline__                              \
    long lkl_sys##name(__LKL__MAP(x, __LKL__SC_DECL, __VA_ARGS__))         \
    {                                  \
        long params[6] = { __LKL__MAP(x, __LKL__SC_LONG, __VA_ARGS__) }; \
        return lkl_syscall(__lkl__NR##name, params);           \
    }

#define LKL_SYSCALL_DEFINE0(name, ...) LKL_SYSCALL0(name)
#define LKL_SYSCALL_DEFINE1(name, ...) LKL_SYSCALLx(1, name, __VA_ARGS__)
#define LKL_SYSCALL_DEFINE2(name, ...) LKL_SYSCALLx(2, name, __VA_ARGS__)
#define LKL_SYSCALL_DEFINE3(name, ...) LKL_SYSCALLx(3, name, __VA_ARGS__)
#define LKL_SYSCALL_DEFINE4(name, ...) LKL_SYSCALLx(4, name, __VA_ARGS__)
#define LKL_SYSCALL_DEFINE5(name, ...) LKL_SYSCALLx(5, name, __VA_ARGS__)
#define LKL_SYSCALL_DEFINE6(name, ...) LKL_SYSCALLx(6, name, __VA_ARGS__)

#define __lkl__NR_chroot 51
LKL_SYSCALL_DEFINE1(_chroot,const char *,filename)

#define __lkl__NR_mknod 1027
typedef unsigned short           lkl_umode_t;
LKL_SYSCALL_DEFINE3(_mknod,const char *,filename,lkl_umode_t,mode,unsigned,dev)

#define __lkl__NR_mkdir 1030
LKL_SYSCALL_DEFINE2(_mkdir,const char *,pathname,umode_t,mode)

#define LKL_MKDEV(ma,mi)    ((ma)<<8 | (mi))

enum rump_etfs_type {
	RUMP_ETFS_REG,
	RUMP_ETFS_BLK,
	RUMP_ETFS_CHR,
	RUMP_ETFS_DIR,
	RUMP_ETFS_DIR_SUBDIRS
};

int rump_pub_etfs_register(const char *, const char *, enum rump_etfs_type);
int rump_pub_etfs_register_withsize(const char *, const char *, enum rump_etfs_type, uint64_t, uint64_t);
int rump_pub_etfs_remove(const char *);

int rump_pub_netconfig_ifcreate(const char *) __attribute__ ((weak));
int rump_pub_netconfig_dhcp_ipv4_oneshot(const char *) __attribute__ ((weak));
int rump_pub_netconfig_auto_ipv6(const char *) __attribute__ ((weak));
int rump_pub_netconfig_ifup(const char *) __attribute__ ((weak));
int rump_pub_netconfig_ipv4_ifaddr(const char *, const char *, const char *) __attribute__ ((weak));
int rump_pub_netconfig_ipv4_ifaddr_cidr(const char *, const char *, int) __attribute__ ((weak));
int rump_pub_netconfig_ipv4_gw(const char *) __attribute__ ((weak));

int rump_pub_netconfig_ifcreate(const char *interface) {return 0;}
int rump_pub_netconfig_dhcp_ipv4_oneshot(const char *interface) {return 0;}
int rump_pub_netconfig_auto_ipv6(const char *interface) {return 0;}
int rump_pub_netconfig_ifup(const char *interface) {return 0;}
int rump_pub_netconfig_ipv4_ifaddr(const char *interface, const char *addr, const char *mask) {return 0;}
int rump_pub_netconfig_ipv4_ifaddr_cidr(const char *interface, const char *addr, int mask) {return 0;};
int rump_pub_netconfig_ipv4_gw(const char *interface) {return 0;}

struct __fdtable __franken_fd[MAXFD];

void
mkkey(char *k, char *n, const char *pre, int dev, int fd)
{
	int i, d;
	int len = strlen(pre);

	if (fd > 99 || dev > 99) abort();
	for (i = 0; i < len; i++)
		*k++ = pre[i];
	if (dev > 9) {
		d = (dev / 10) + '0';
		*k++ = d;
		dev /= 10;
	}
	d = dev + '0';
	*k++ = d;
	*k++ = 0;

	if (fd > 9) {
		d = (fd / 10) + '0';
		*n++ = d;
		fd /= 10;
	}
	d = fd + '0';
	*n++ = d;
	*n++ = 0;
}

void
__franken_fdinit()
{
	int fd;
	struct stat st;

	/* iterate over numbered descriptors, stopping when one does not exist */
	for (fd = 0; fd < MAXFD; fd++) {
		memset(&st, 0, sizeof(struct stat));
		if (fstat(fd, &st) == -1) {
			__franken_fd[fd].valid = 0;
			break;
		}
		__franken_fd[fd].valid = 1;
		__franken_fd[fd].flags = fcntl(fd, F_GETFL, 0);
		memcpy(&__franken_fd[fd].st, &st, sizeof(struct stat));
		/* XXX move this to platform code */
		switch (st.st_mode & S_IFMT) {
		case S_IFREG:
			__franken_fd[fd].seek = 1;
			break;
		case S_IFBLK:
			__franken_fd[fd].seek = 1;
#ifdef MUSL_LIBC
			/* notify virtio-mmio dev id */
			union lkl_disk disk;
			disk.fd = fd;
			disk_id = lkl_disk_add(disk);
#endif
			break;
		case S_IFCHR:
			/* XXX Linux presents stdin as char device see notes to clean up */
			__franken_fd[fd].seek = 0;
			break;
		case S_IFIFO:
			__franken_fd[fd].seek = 0;
			break;
		case S_IFSOCK:
			__franken_fd[fd].seek = 0;
#ifdef MUSL_LIBC
			/* notify virtio-mmio dev id */
			union lkl_netdev nd;
			nd.fd = fd;
			nd_id = lkl_netdev_add(nd, NULL);
#endif
			break;
		}
	}
}

/* XXX would be much nicer to build these functions against NetBSD libc headers, but at present
   they are not built, or installed, yet. Need to reorder the build process */

#define IOCPARM_MASK    0x1fff
#define IOCPARM_SHIFT   16
#define IOCGROUP_SHIFT  8
#define IOCPARM_LEN(x)  (((x) >> IOCPARM_SHIFT) & IOCPARM_MASK)
#define IOCBASECMD(x)   ((x) & ~(IOCPARM_MASK << IOCPARM_SHIFT))
#define IOCGROUP(x)     (((x) >> IOCGROUP_SHIFT) & 0xff)

#define IOC_VOID        (unsigned long)0x20000000
#define IOC_OUT         (unsigned long)0x40000000
#define IOC_IN          (unsigned long)0x80000000
#define IOC_INOUT       (IOC_IN|IOC_OUT)
#define IOC_DIRMASK     (unsigned long)0xe0000000

#define _IOC(inout, group, num, len) \
    ((inout) | (((len) & IOCPARM_MASK) << IOCPARM_SHIFT) | \
    ((group) << IOCGROUP_SHIFT) | (num))
#define _IO(g,n)        _IOC(IOC_VOID,  (g), (n), 0)
#define _IOR(g,n,t)     _IOC(IOC_OUT,   (g), (n), sizeof(t))
#define _IOW(g,n,t)     _IOC(IOC_IN,    (g), (n), sizeof(t))
#define _IOWR(g,n,t)    _IOC(IOC_INOUT, (g), (n), sizeof(t))

struct ufs_args {
	char *fspec;
};

struct tmpfs_args {
	int ta_version;
	ino_t ta_nodes_max;
	off_t ta_size_max;
	uid_t ta_root_uid;
	gid_t ta_root_gid;
	mode_t ta_root_mode;
};

#define MNT_RDONLY	0x00000001
#define MNT_LOG		0x02000000
#define MNT_FORCE	0x00080000

#define IFNAMSIZ 16

struct ifcapreq {
	char		ifcr_name[IFNAMSIZ];
	uint64_t	ifcr_capabilities;
	uint64_t	ifcr_capenable;
};

#define SIOCGIFCAP	_IOWR('i', 118, struct ifcapreq)
#define SIOCSIFCAP	_IOW('i', 117, struct ifcapreq)

int rump___sysimpl_open(const char *, int, ...);
int rump___sysimpl_close(int);
int rump___sysimpl_dup2(int, int);
int rump___sysimpl_mount50(const char *, const char *, int, void *, size_t);
int rump___sysimpl_unmount(const char *, int);
int rump___sysimpl_socket30(int, int, int);
int rump___sysimpl_mkdir(const char *, mode_t);
int rump___sysimpl_ioctl(int, unsigned long, void *);

#define AF_UNSPEC 0
#define AF_INET 2
#define AF_INET6 24
#define SOCK_STREAM 1

static void
mount_tmpfs(void)
{
	struct tmpfs_args ta = {
		.ta_version = 1,
		.ta_nodes_max = 0,
		.ta_size_max = 0,
		.ta_root_uid = 0,
		.ta_root_gid = 0,
		.ta_root_mode = 0777,
	};

	/* try to mount tmpfs if possible, useful for ro root fs */
	rump___sysimpl_mkdir("/tmp", 0777);
	/* might fail if no /tmp directory, or tmpfs not included in system */
	rump___sysimpl_mount50("tmpfs", "/tmp", 0, &ta, sizeof(struct tmpfs_args));
}

static void
unmount_atexit(void)
{
	int ret __attribute__((__unused__));

#ifdef MUSL_LIBC
	ret = lkl_umount_dev("/etc", 0);
#else
	ret = rump___sysimpl_unmount("/", MNT_FORCE);
#endif
}

static int
register_reg(int dev, int fd, int flags)
{
	char key[16], num[16];

	mkkey(key, num, "/dev/vfile", dev, fd);
	rump_pub_etfs_register(key, num, RUMP_ETFS_REG);
	return rump___sysimpl_open(key, flags);
}

static void
register_net(int fd)
{
#ifdef MUSL_LIBC
	/* FIXME: hehe always fixme tagged.. */
	lkl_if_up(lkl_netdev_get_ifindex(nd_id));
	lkl_if_set_ipv4(lkl_netdev_get_ifindex(nd_id), 0x0200010a /* 10.1.0.2 */,
			24);
#if 0
	lkl_if_set_ipv4(lkl_netdev_get_ifindex(nd_id), 0x0cd1a8c0 /* 192.168.209.12 */,
			24);
#endif
#else
	char key[16], num[16];
	int ret;
	int sock;
	char *addr, *mask, *gw;
	int af = AF_UNSPEC;
	struct ifcapreq cap;

	mkkey(key, num, "virt", fd, fd);
	ret = rump_pub_netconfig_ifcreate(key);
	if (ret == 0) {
		sock = rump___sysimpl_socket30(AF_INET6, SOCK_STREAM, 0);
		if (sock != -1) {
			rump_pub_netconfig_auto_ipv6(key);
			rump___sysimpl_close(sock);
			af = AF_INET6;
		}
		sock = rump___sysimpl_socket30(AF_INET, SOCK_STREAM, 0);
		if (sock != -1) {
			/* XXX move to autodetect later, but gateway complex */
			addr = getenv("FIXED_ADDRESS");
			mask = getenv("FIXED_MASK");
			gw = getenv("FIXED_GATEWAY");
			if (addr == NULL || mask == NULL || gw == NULL) {
				rump_pub_netconfig_dhcp_ipv4_oneshot(key);
			} else {
				rump_pub_netconfig_ifup(key);
				rump_pub_netconfig_ipv4_ifaddr_cidr(key, addr, atoi(mask));
				rump_pub_netconfig_ipv4_gw(gw);
			}
			rump___sysimpl_close(sock);
			af = AF_INET;
		}
		/* enable offload features */
		sock = rump___sysimpl_socket30(af, SOCK_STREAM, 0);
		if (sock != -1) {
			strncpy(&cap.ifcr_name, key, IFNAMSIZ);
			ret = rump___sysimpl_ioctl(sock, SIOCGIFCAP, &cap);
			if (ret != -1) {
				cap.ifcr_capenable = cap.ifcr_capabilities;
				rump___sysimpl_ioctl(sock, SIOCSIFCAP, &cap);
			}
			rump___sysimpl_close(sock);
		}
	}
#endif
}

static int
register_block(int dev, int fd, int flags, off_t size, int root)
{
#ifdef MUSL_LIBC
	/* FIXME: hehe always fixme tagged.. */
	int ret;
	char mnt_point[32] = "/etc";

	ret = lkl_mount_dev(disk_id, "ext4", 0, NULL, mnt_point,
			    sizeof(mnt_point));
	if (ret < 0)
		printf("can't mount disk (%d) at %s. err=%d\n",
			disk_id, mnt_point, ret);

    lkl_sys_chroot("/etc");
    lkl_sys_mkdir("/dev", 0700);
    lkl_sys_mknod("/dev/null", 0666, LKL_MKDEV(1, 3));

	atexit(unmount_atexit);
	return ret;
#else
	char key[16], rkey[16], num[16];
	struct ufs_args ufs;
	int ret;

	mkkey(key, num, "/dev/block", dev, fd);
	mkkey(rkey, num, "/dev/rblock", dev, fd);
	rump_pub_etfs_register_withsize(key, num, RUMP_ETFS_BLK, 0, size);
	rump_pub_etfs_register_withsize(rkey, num, RUMP_ETFS_CHR, 0, size);
	if (root == 0)
		return 0;
	ufs.fspec = key;
	if (flags == O_RDWR)
		flags = MNT_LOG;
	else
		flags = MNT_RDONLY;
	ret = rump___sysimpl_mount50("ffs", "/", flags, &ufs, sizeof(struct ufs_args));
	if (ret == -1) {
		if (flags == MNT_LOG)
			flags = 0;
		ret = rump___sysimpl_mount50("ext2fs", "/", flags, &ufs, sizeof(struct ufs_args));
	}
	if (ret == 0)
		atexit(unmount_atexit);
	return ret;
#endif
}

void
__franken_fdinit_create()
{
	int fd;
	int n_reg = 0, n_block = 0;

	if (__franken_fd[0].valid) {
		fd = register_reg(n_reg++, 0, O_RDONLY);
		if (fd != -1) {
			rump___sysimpl_dup2(fd, 0);
			rump___sysimpl_close(fd);
		}
	}
	if (__franken_fd[1].valid) {
		fd = register_reg(n_reg++, 1, O_WRONLY);
		if (fd != -1) {
			rump___sysimpl_dup2(fd, 1);
			rump___sysimpl_close(fd);
		}
	}

	if (__franken_fd[2].valid) {
		fd = register_reg(n_reg++, 2, O_WRONLY);
		if (fd != -1) {
			rump___sysimpl_dup2(fd, 2);
			rump___sysimpl_close(fd);
		}
	}

	/* XXX would be nicer to be able to detect a file system,
	   but this also allows us not to mount a block device.
	   Pros and cons, may change if this is not convenient */

	/* only fd 3 will be mounted as root file system */
	if (__franken_fd[3].valid) {
		fd = 3;
		switch (__franken_fd[fd].st.st_mode & S_IFMT) {
		case S_IFREG:
		case S_IFBLK:
			if (register_block(n_block++, fd,
			    __franken_fd[fd].flags & O_ACCMODE,
			    __franken_fd[fd].st.st_size, 1) == 0) {
				__franken_fd[fd].mounted = 1;
			}
			break;
		case S_IFSOCK:
			register_net(fd);
			break;
		}
	}

	for (fd = 4; fd < MAXFD; fd++) {
		if (__franken_fd[fd].valid == 0)
			break;
		switch (__franken_fd[fd].st.st_mode & S_IFMT) {
		case S_IFREG:
			fd = register_reg(n_reg++, fd, __franken_fd[fd].flags & O_ACCMODE);
			break;
		case S_IFBLK:
			register_block(n_block++, fd, __franken_fd[fd].flags & O_ACCMODE, __franken_fd[fd].st.st_size, 0);
			break;
		case S_IFSOCK:
			register_net(fd);
			break;
		}
	}

	/* now some generic stuff */
	mount_tmpfs();
}
