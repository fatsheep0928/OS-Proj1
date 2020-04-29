#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage long sys_my_printk(int pid, long a, long b, long c, long d){
    printk("[project 1] %d %ld.%ld %ld.%ld\n", pid, a, b, c, d);
    return 1;
}
