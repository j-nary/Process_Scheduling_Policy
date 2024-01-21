#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

asmlinkage long long sys_reverse_order(const char __user *user_input, char __user *user_output, size_t len) {
    char *reversed_str;
    int i;
    reversed_str = kmalloc(len + 1, GFP_KERNEL);

    if (copy_from_user(reversed_str, user_input, len)) {
        kfree(reversed_str);
        return -EFAULT;
    }

    reversed_str[len] = '\0';
   
    for (i = 0; i < len / 2; i++) {
        char temp = reversed_str[i];
        reversed_str[i] = reversed_str[len - i - 1];
        reversed_str[len - i - 1] = temp;
    }

    if (copy_to_user(user_output, reversed_str, len)) {
        kfree(reversed_str);
        return -EFAULT;
    }

    kfree(reversed_str);
    return 0;
}

SYSCALL_DEFINE3(reverse_order, const char __user *, user_input, char __user *, user_output, size_t, len) {
    return sys_reverse_order(user_input, user_output, len);
}

