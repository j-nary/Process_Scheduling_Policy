#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long long sys_sub(const long long a, const long long b) {
	long long ret = a - b;
	printk("Sub Result : %lld\n", ret);
	return ret;
}
SYSCALL_DEFINE2(sub, const long long, a, const long long, b) {
	return sys_sub(a, b);
}
