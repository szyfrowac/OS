#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

SYSCALL_DEFINE0(myfork)
{
    long ret;
    pr_info("myfork: Process %d is cloning itself.\n", current->pid);

    ret = kernel_clone(&(struct kernel_clone_args){
        .flags = SIGCHLD,
        .exit_signal = SIGCHLD,
    });

    return ret;
}
