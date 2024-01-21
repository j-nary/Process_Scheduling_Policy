#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long long sys_add(const long long a, const long long b) {
	long long ret = a + b;
	printk("Add Result : %lld\n", ret);
	return ret;
}
SYSCALL_DEFINE2(add, const long long, a, const long long, b) {
	return sys_add(a, b);
}

