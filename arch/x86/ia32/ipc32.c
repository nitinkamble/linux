#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/ipc.h>
#include <linux/compat.h>
#include <asm/sys_ia32.h>

asmlinkage long sys32_ipc(u32 call, int first, int second, int third,
			  compat_uptr_t ptr, u32 fifth)
{
	int version;

	version = call >> 16; /* hack for backward compatibility */
	call &= 0xffff;

	switch (call) {
	case SEMOP:
		/* struct sembuf is the same on 32 and 64bit :)) */
		return sys_semtimedop(first, compat_ptr(ptr), second, NULL);
	case SEMTIMEDOP:
		return compat_sys_semtimedop(first, compat_ptr(ptr), second,
						compat_ptr(fifth));
	case SEMGET:
		return sys_semget(first, second, third);
	case SEMCTL:
		return compat_sys_semctl(first, second, third, compat_ptr(ptr));

	case MSGSND:
		return compat_sys_msgsnd(first, second, third, compat_ptr(ptr));
	case MSGRCV:
		return compat_sys_msgrcv(first, second, fifth, third,
					 version, compat_ptr(ptr));
	case MSGGET:
		return sys_msgget((key_t) first, second);
	case MSGCTL:
		return compat_sys_msgctl(first, second, compat_ptr(ptr));

	case SHMAT:
		return compat_sys_shmat(first, second, third, version,
					compat_ptr(ptr));
	case SHMDT:
		return sys_shmdt(compat_ptr(ptr));
	case SHMGET:
		return sys_shmget(first, (unsigned)second, third);
	case SHMCTL:
		return compat_sys_shmctl(first, second, compat_ptr(ptr));
	}
	return -ENOSYS;
}

#ifdef CONFIG_X86_X32_ABI
/* Already defined in ipc/compat.c, but we need it here. */
struct compat_msgbuf {
	compat_long_t mtype;
	char mtext[1];
};

long compat_sys_x32_msgrcv(int first, void __user *uptr, int second,
			   int msgtyp, int third)
{
	struct compat_msgbuf __user *up;
	long type;
	int err;

	if (first < 0)
		return -EINVAL;
	if (second < 0)
		return -EINVAL;

	up = uptr;
	err = do_msgrcv(first, &type, up->mtext, second, msgtyp, third);
	if (err < 0)
		goto out;
	if (put_user(type, &up->mtype))
		err = -EFAULT;
out:
	return err;
}

long compat_sys_x32_msgsnd(int first, void __user *uptr, int second,
			   int third)
{
	struct compat_msgbuf __user *up = uptr;
	long type;

	if (first < 0)
		return -EINVAL;
	if (second < 0)
		return -EINVAL;

	if (get_user(type, &up->mtype))
		return -EFAULT;

	return do_msgsnd(first, type, up->mtext, second, third);
}

long compat_sys_x32_shmat(int first, void __user *uptr, int second)
{
	int err;
	unsigned long raddr;

	err = do_shmat(first, uptr, second, &raddr);
	if (err < 0)
		return err;
	return (long) raddr;
}

int compat_sys_x32_semctl(int semid, int semnum, int cmd, u32 arg)
{
	/* compat_sys_semctl expects a pointer to union semun */
	u32 __user *uptr = compat_alloc_user_space(sizeof(u32));
	if (put_user(arg, uptr))
		return -EFAULT;
	return compat_sys_semctl(semid, semnum, cmd, uptr);
}
#endif
