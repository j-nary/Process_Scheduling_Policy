#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>

asmlinkage long long sys_print_cpu_burst(const int pid) {
	struct pid *pid_struct;
	struct task_struct *task;
	pid_struct = find_get_pid(pid);
	if (!pid_struct)
		return -1;
	task = pid_task(pid_struct, PIDTYPE_PID);
	if (!task) {
		printk(KERN_INFO "PID %d Fail", pid);
		put_pid(pid_struct);
		return -1;
	}

	unsigned long long total_utime = task -> utime;
	unsigned long long total_stime = task -> stime;
	unsigned long long total_time_in_sec = (total_utime +  total_stime) / HZ;
	printk(KERN_INFO "[pid : %d] CPU burst : %llu\n", pid, total_time_in_sec);
	
	put_pid(pid_struct);
	return 0;
}
SYSCALL_DEFINE1(print_cpu_burst, int, pid) {
	return sys_print_cpu_burst(pid);
}
