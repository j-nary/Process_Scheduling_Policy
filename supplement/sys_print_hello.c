#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_print_hello(void) {
	printk("Hello Wolrd!\n");
	return 0;
}
SYSCALL_DEFINE0(print_hello) {
	return sys_print_hello();
}
