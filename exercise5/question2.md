依据自己的实验选择，分析uCore或rCore中下面执行过程，并形成文档。
1. 批处理操作系统中应用程序管理数据结构的组成；
2. 应用程序管理数据结构的初始化过程；
3. trapframe数据结构的组成；
4. 在系统调用过程中的trapframe数据结构的保存过程；
5. 在系统调用返回过程中的从trapframe数据结构恢复应用程序执行上下文的过程；
6. 系统调用执行过程中的参数和返回值传递过程；

# 批处理操作系统中应用程序管理数据结构的组成
在link_apps.S中存放了app的总数量，每个app的信息地址
```asm
_app_num:
    .quad 40 # app的总数量
    .quad app_0_start # app0的地址
    ...

    .global _app_names 
_app_names: # app的名字
   .string "ch2b_exit"
...
.global INIT_PROC
INIT_PROC:
    .string "usershell"

    .section .data.app0 # app0的代码段
    .global app_0_start # app0的入口地址
app_0_start:
    .incbin "./user/target/bin/ch2b_exit" # app0的二进制文件
```
在加载进程时，先从_app_num找到当前app和下一个app的地址(或者结尾地址)，然后将中间区域，也就是app的二进制文件加载到内存中，切换上下文运行即可

# 应用程序管理数据结构的初始化过程
初始化在loader.c中，将指针指向_app_num，读取app的总数量，初始化当前app的索引
```c
void loader_init()
{
	if ((uint64)ekernel >= BASE_ADDRESS) {
		panic("kernel too large...\n");
	}
	app_info_ptr = (uint64 *)_app_num; // app_info_ptr指向_app_num
	app_cur = -1;
	app_num = *app_info_ptr; // 读取出app的总数量
}
```
在顺次运行时：
```c
int run_next_app()
{
	struct trapframe *trapframe = (struct trapframe *)trap_page;
	app_cur++; // 当前app的索引加1
	app_info_ptr++; // app_info_ptr指向下一个app的地址
	if (app_cur >= app_num) {
		return -1;
	}
	infof("load and run app %d", app_cur);
	uint64 length = load_app(app_info_ptr); // 加载app
	debugf("bin range = [%p, %p)", *app_info_ptr, *app_info_ptr + length);
	memset(trapframe, 0, 4096);
	trapframe->epc = BASE_ADDRESS;
	trapframe->sp = (uint64)user_stack + USER_STACK_SIZE;
	usertrapret(trapframe, (uint64)boot_stack_top);
	return 0;
}
```

# trapframe数据结构的组成
```c
struct trapframe {
	/*   0 */ uint64 kernel_satp; // kernel page table
	/*   8 */ uint64 kernel_sp; // top of process's kernel stack
	/*  16 */ uint64 kernel_trap; // usertrap()
	/*  24 */ uint64 epc; // saved user program counter
	/*  32 */ uint64 kernel_hartid; // saved kernel tp
	/*  40 */ uint64 ra;
	/*  48 */ uint64 sp;
	/*  56 */ uint64 gp;
	/*  64 */ uint64 tp;
	/*  72 */ uint64 t0;
	/*  80 */ uint64 t1;
	/*  88 */ uint64 t2;
	/*  96 */ uint64 s0;
	/* 104 */ uint64 s1;
	/* 112 */ uint64 a0;
	/* 120 */ uint64 a1;
	/* 128 */ uint64 a2;
	/* 136 */ uint64 a3;
	/* 144 */ uint64 a4;
	/* 152 */ uint64 a5;
	/* 160 */ uint64 a6;
	/* 168 */ uint64 a7;
	/* 176 */ uint64 s2;
	/* 184 */ uint64 s3;
	/* 192 */ uint64 s4;
	/* 200 */ uint64 s5;
	/* 208 */ uint64 s6;
	/* 216 */ uint64 s7;
	/* 224 */ uint64 s8;
	/* 232 */ uint64 s9;
	/* 240 */ uint64 s10;
	/* 248 */ uint64 s11;
	/* 256 */ uint64 t3;
	/* 264 */ uint64 t4;
	/* 272 */ uint64 t5;
	/* 280 */ uint64 t6;
};
```
trapframe结构体保存了：
1. 内核态的页表
2. 内核栈指针
3. 内核trap处理函数地址
4. 用户态程序计数器
5. 内核线程指针
6. 用户态的寄存器

# 在系统调用过程中的trapframe数据结构的保存过程
```asm
.globl trampoline
trampoline:
.align 4
.globl uservec
uservec:
	#
        # trap.c sets stvec to point here, so
        # traps from user space start here,
        # in supervisor mode, but with a
        # user page table.
        #
        # sscratch points to where the process's p->trapframe is
        # mapped into user space, at TRAPFRAME.
        #

	# swap a0 and sscratch
        # so that a0 is TRAPFRAME
        csrrw a0, sscratch, a0 # 将trapframe的地址存入a0

        # save the user registers in TRAPFRAME
        sd ra, 40(a0) # 将用户态的寄存器保存到trapframe中
        sd sp, 48(a0)
        sd gp, 56(a0)
        sd tp, 64(a0)
        sd t0, 72(a0)
        sd t1, 80(a0)
        sd t2, 88(a0)
        sd s0, 96(a0)
        sd s1, 104(a0)
        sd a1, 120(a0)
        sd a2, 128(a0)
        sd a3, 136(a0)
        sd a4, 144(a0)
        sd a5, 152(a0)
        sd a6, 160(a0)
        sd a7, 168(a0)
        sd s2, 176(a0)
        sd s3, 184(a0)
        sd s4, 192(a0)
        sd s5, 200(a0)
        sd s6, 208(a0)
        sd s7, 216(a0)
        sd s8, 224(a0)
        sd s9, 232(a0)
        sd s10, 240(a0)
        sd s11, 248(a0)
        sd t3, 256(a0)
        sd t4, 264(a0)
        sd t5, 272(a0)
        sd t6, 280(a0)

	# save the user a0 in p->trapframe->a0
        csrr t0, sscratch # t0 = sscratch，就是之前换到的a0
        sd t0, 112(a0) # 保存用户态的a0

        csrr t1, sepc
        sd t1, 24(a0)

        ld sp, 8(a0)
        ld tp, 32(a0)
        ld t1, 0(a0)
        # csrw satp, t1
        # sfence.vma zero, zero
        ld t0, 16(a0)
        jr t0
```
在uservec中，首先将sscratch寄存器中的trapframe地址保存到a0中，然后将用户态的寄存器依次保存到trapframe中，最后跳转到内核态的trap处理函数

# 在系统调用返回过程中的从trapframe数据结构恢复应用程序执行上下文的过程
```asm
.globl userret
userret:
        # userret(TRAPFRAME, pagetable)
        # switch from kernel to user.
        # usertrapret() calls here.
        # a0: TRAPFRAME, in user page table.
        # a1: user page table, for satp.

        # switch to the user page table.
        # csrw satp, a1
        # sfence.vma zero, zero

        # put the saved user a0 in sscratch, so we
        # can swap it with our a0 (TRAPFRAME) in the last step.
        ld t0, 112(a0) # 从trapframe中加载用户态的a0
        csrw sscratch, t0 # 将用户态的a0保存到sscratch中

        # restore all but a0 from TRAPFRAME
        ld ra, 40(a0) # 从trapframe中加载用户态的寄存器
        ld sp, 48(a0)
        ld gp, 56(a0)
        ld tp, 64(a0)
        ld t0, 72(a0)
        ld t1, 80(a0)
        ld t2, 88(a0)
        ld s0, 96(a0)
        ld s1, 104(a0)
        ld a1, 120(a0)
        ld a2, 128(a0)
        ld a3, 136(a0)
        ld a4, 144(a0)
        ld a5, 152(a0)
        ld a6, 160(a0)
        ld a7, 168(a0)
        ld s2, 176(a0)
        ld s3, 184(a0)
        ld s4, 192(a0)
        ld s5, 200(a0)
        ld s6, 208(a0)
        ld s7, 216(a0)
        ld s8, 224(a0)
        ld s9, 232(a0)
        ld s10, 240(a0)
        ld s11, 248(a0)
        ld t3, 256(a0)
        ld t4, 264(a0)
        ld t5, 272(a0)
        ld t6, 280(a0)

	# restore user a0, and save TRAPFRAME in sscratch
        csrrw a0, sscratch, a0 # 将sscratch中的用户态a0保存到a0中，同时将trapframe地址保存到sscratch中

        # return to user mode and user pc.
        # usertrapret() set up sstatus and sepc.
        sret
```
在userret中，首先将trapframe中的用户态a0保存到t0中，然后将t0保存到sscratch中，接着从trapframe中依次加载用户态的寄存器，最后将sscratch中的用户态a0保存到a0中，同时将trapframe地址保存到sscratch中，最后执行sret指令返回用户态

# 系统调用执行过程中的参数和返回值传递过程
```c
void syscall()
{
	struct trapframe *trapframe = (struct trapframe *)trap_page;
	int id = trapframe->a7, ret;
	uint64 args[6] = { trapframe->a0, trapframe->a1, trapframe->a2,
			   trapframe->a3, trapframe->a4, trapframe->a5 };
	tracef("syscall %d args = [%x, %x, %x, %x, %x, %x]", id, args[0],
	       args[1], args[2], args[3], args[4], args[5]);
	switch (id) {
	case SYS_write:
		ret = sys_write(args[0], (char *)args[1], args[2]);
		break;
	case SYS_exit:
		sys_exit(args[0]);
		// __builtin_unreachable();
	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
```
系统调用的参数存储在a0-a5中，系统调用号存储在a7中，系统调用返回值存储在a0中。在syscall函数中，首先从trapframe中获取系统调用号和参数，然后根据系统调用号调用相应的系统调用函数，最后将返回值存储在trapframe的a0中（注意不是真的a0寄存器）。这样在返回用户态时，用户态的a0中就存储了系统调用的返回值。