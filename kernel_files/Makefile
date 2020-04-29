version = 4.16.7
#version = 4.15.13
all:
	cp my_printk.c ~/linux-${version}/kernel/.
	rm -f ~/linux-${version}/kernel/Makefile
	cp mf ~/linux-${version}/kernel/Makefile
	cp syscall_64.tbl ~/linux-${version}/arch/x86/entry/syscalls/.
	cp syscalls.h ~/linux-${version}/include/linux/.
	cd ~/linux-${version}
get:
	cp ~/linux-${version}/kernel/my_printk.c .
	cp ~/linux-${version}/kernel/Makefile ./mf
	cp ~/linux-${version}/arch/x86/entry/syscalls/syscall_64.tbl .
	cp ~/linux-${version}/include/linux/syscalls.h .
