# Linux驱动入门-驱动

## 1. 引子

驱动是干什么的？在驱动的相关书籍上，网络上你能看到很多专业的定义。我们暂时不关心这些专业的说法，仅从功能的角度来说，驱动程序使得应用程序可以访问硬件。

那应用是如何访问硬件的？linux 中一切皆文件，访问硬件就是对文件的读写操作。比如 led 灯对应的文件是  /etc/led, 读写这个文件就能操作 led 灯。

接下来的问题就是，linux 中如何读写文件？

## 2. linux 中文件的读写

linux中文件读写相关的主要 api：

```c
//打开文件
int open(const char *pathname, int flags, mode_t mode);
//从文件中读数据
ssize_t read(int fd, void *buf, size_t count);
//向文件中写数据
ssize_t write(int fd, const void *buf, size_t count);
//专用于设备输入输出操作
int ioctl(int fd, unsigned long request, ...);
//关闭文件的读写，回收资源
int close(int fd);
```

函数的具体用法不是本文的重点，有兴趣的同学可以学习 [Linux程序设计](https://book.douban.com/subject/4831448/) 的第二章。工作中忘了，可以通过 man 命名查看具体用法。

下面来看一下 open 函数：

```c
//该函数用于打开文件
int open(const char *pathname, int flags, mode_t mode);
```

当打开一个文件的时候，会返回一个 int 值，一般称这个返回值为句柄或者 handle，查看内核代码可以发现：句柄是一个数组的 index。数值的成员是 struct file ：

```c
struct file {
        union {
                struct llist_node       fu_llist;
                struct rcu_head         fu_rcuhead;
        } f_u;
        struct path             f_path;
        struct inode            *f_inode;       /* cached value */
        const struct file_operations    *f_op;   //关注1

        /*
         * Protects f_ep_links, f_flags.
         * Must not be taken from IRQ context.
         */
        spinlock_t              f_lock;
        enum rw_hint            f_write_hint;
        atomic_long_t           f_count;
        unsigned int            f_flags;  //关注2
        fmode_t                 f_mode;	  //关注3
        struct mutex            f_pos_lock;
        loff_t                  f_pos;	  //关注4
        struct fown_struct      f_owner;
        const struct cred       *f_cred;
        struct file_ra_state    f_ra;

        u64                     f_version;
#ifdef CONFIG_SECURITY
        void                    *f_security;
#endif
        /* needed for tty driver, and maybe others */
        void                    *private_data;

#ifdef CONFIG_EPOLL
        /* Used by fs/eventpoll.c to link all the hooks to this file */
        struct list_head        f_ep_links;
        struct list_head        f_tfile_llink;
#endif /* #ifdef CONFIG_EPOLL */
        struct address_space    *f_mapping;
        errseq_t                f_wb_err;
} __randomize_layout
  __attribute__((aligned(4)));
```

struct file 的结构有点复杂，入门阶段主要关注代码中标注的四个关注点。

在内核中，有一个 struct file 的数组，当调用 open 函数打开一个文件的时候，内核就会构建一个 struct file，并添加到这个数组中，返回 struct  file 的 index 给用户态程序。

根据文件的命名，容易猜出：使用open打开文件时，传入的 flags、mode 等参数会被记录在内核中，具体如下图所示：

![image.png](https://cdn.nlark.com/yuque/0/2022/png/2613680/1662693810943-6a800425-31f9-47d2-84db-49b2908a3d31.png#clientId=u9b7a7c40-86b9-4&crop=0&crop=0&crop=1&crop=1&from=paste&height=673&id=u337b83ee&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1010&originWidth=1743&originalType=binary&ratio=1&rotation=0&showTitle=false&size=254004&status=done&style=none&taskId=u1b205f35-f635-4e83-9d1b-e3f0994d834&title=&width=1162)

struct file 有一个成员为 file_operations：

```c
struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);  //关注点1
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); //关注点2
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iterate) (struct file *, struct dir_context *);
	int (*iterate_shared) (struct file *, struct dir_context *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);//关注点3
	int (*mmap) (struct file *, struct vm_area_struct *);//关注点4
	int (*open) (struct inode *, struct file *);//关注点5
	int (*flush) (struct file *, fl_owner_t id);//关注点6
	int (*release) (struct inode *, struct file *);//关注点7
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
			loff_t, size_t, unsigned int);
	int (*clone_file_range)(struct file *, loff_t, struct file *, loff_t,
			u64);
	ssize_t (*dedupe_file_range)(struct file *, u64, u64, struct file *,
			u64);
} __randomize_layout;
```

内部主要是一些函数指针，我们主要关注常用的几个函数：

```c
ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
int (*mmap) (struct file *, struct vm_area_struct *);
int (*mmap) (struct file *, struct vm_area_struct *);
int (*open) (struct inode *, struct file *);
int (*flush) (struct file *, fl_owner_t id);
int (*release) (struct inode *, struct file *);
```

这些函数都是由相应硬件驱动提供的。

至此，文件读写的大致流程就出来了：

- app 调用 open read 等系统调用函数
- 内核构建相应的 struct file，并添加进数组，返回 index 给 app
- 调用驱动程序提供的 open read 等函数，完成实际的硬件操作

## 3. hello 驱动的编写

驱动就是一个模块，在模块的基础上添加驱动框架和硬件操作的部分就可以完成驱动程序的编写了。下面我们写一个 hello 驱动，这个驱动只是简单的在用户态和内核态之间拷贝数据，没有实际的硬件操作，仅用于流程的展示。编写驱动的步骤如下：

1. 确定主设备号，也可以让内核分配 （设备号就是硬件的一个编号）
2. 定义自己的 file_operations 结构体
3. 实现对应的 drv_open/drv_read/drv_write 等函数，填入 file_operations 结构体
4. 定义 init 函数，在 init 函数中调用 register_chrdev 注册函数
5. 定义 exit 函数，在 exit 函数中调用 unregister_chrdev 卸载函数
6. 其他完善：提供设备信息，自动创建设备节点：class_create, device_create

```c
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>

/* 1. 确定主设备号                                                                 */
static int major = 0;
static char kernel_buf[1024];
static struct class *hello_class;


#define MIN(a, b) (a < b ? a : b)

/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t hello_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_to_user(buf, kernel_buf, MIN(1024, size));
	return MIN(1024, size);
}

static ssize_t hello_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(kernel_buf, buf, MIN(1024, size));
	return MIN(1024, size);
}

static int hello_drv_open (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int hello_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 2. 定义自己的file_operations结构体                                              */
static struct file_operations hello_drv = {
	.owner	 = THIS_MODULE,
	.open    = hello_drv_open,
	.read    = hello_drv_read,
	.write   = hello_drv_write,
	.release = hello_drv_close,
};

/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init hello_init(void)
{
	int err;

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "hello", &hello_drv);  /* /dev/hello */

	//提供设备信息，自动创建设备节点。
	hello_class = class_create(THIS_MODULE, "hello_class");
	err = PTR_ERR(hello_class);
	if (IS_ERR(hello_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "hello");
		return -1;
	}

	device_create(hello_class, NULL, MKDEV(major, 0), NULL, "hello"); /* /dev/hello */
	//到这里我们就可以通过 /dev/hello 文件来访问我们的驱动程序了。
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit hello_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(hello_class, MKDEV(major, 0));
	class_destroy(hello_class);
	unregister_chrdev(major, "hello");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");

```

接着我们再写一个测试程序：

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * ./hello_drv_test -w abc
 * ./hello_drv_test -r
 */
int main(int argc, char **argv)
{
	int fd;
	char buf[1024];
	int len;

	/* 1. 判断参数 */
	if (argc < 2) 
	{
		printf("Usage: %s -w <string>\n", argv[0]);
		printf("       %s -r\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open("/dev/hello", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/hello\n");
		return -1;
	}

	/* 3. 写文件或读文件 */
	if ((0 == strcmp(argv[1], "-w")) && (argc == 3))
	{
		len = strlen(argv[2]) + 1;
		len = len < 1024 ? len : 1024;
		write(fd, argv[2], len);
	}
	else
	{
		len = read(fd, buf, 1024);	
		buf[1023] = '\0';
		printf("APP read : %s\n", buf);
	}

	close(fd);

	return 0;
}


```

我们可以按照介绍的方法将驱动编译进内核。也可以直接编写 makefile 来编译驱动模块，然后通过命令行加载和卸载驱动程序。这里介绍第二种方法：

创建 Makefile 文件：

```makefile
KERN_DIR = /home/zzh0838/kernel/goldfish

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o hello_drv_test hello_drv_test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f hello_drv_test

obj-m	+= hello_drv.o
```

-C  选项的作用是指将当前工作目录转移到你所指定的位置。“M=”选项的作用是，当用户需要以某个内核为基础编译一个外部模块的话，需要在 make modules  命令中加入“M=dir”，程序会自动到你所指定的 dir 目录中查找模块源码，将其编译，生成 KO 文件。

编写编译驱动的脚本 build_driver.sh：

```bash
#!/bin/bash
export ARCH=x86_64
export SUBARCH=x86_64
export CROSS_COMPILE=x86_64-linux-android-
export PATH=~/aosp/prebuilts/gcc/linux-x86/x86/x86_64-linux-android-4.9/bin:$PATH
make
```

执行 ./build_driver.sh，编译出 hello_drv.ko，接下来启动模拟器，把 ko 文件上传到模拟器：

```bash
cd aosp目录
source build/envsetup.sh
lunch aosp_x86_64-eng
emulator -kernel ~/kernel/goldfish/arch/x86_64/boot/bzImage

cd hellodriver目录
# 使用 adb 上传 ko 文件
adb push hello_drv.ko hello_drv_test /data/local/tmp
# 进入模拟器的 shell 环境
adb shell 
cd /data/local/tmp
#加载模块，加载完成后，/dev 目录下就会有一个 hello 文件
insmod hello_drv.ko
ls /dev/hello -l
```

编译测试程序:
编写 CMakefileLists.txt:

```cmake
cmake_minimum_required(VERSION 3.0)

project(test)

add_executable(${PROJECT_NAME} hello_drv_test.c)
```

编写编译脚本 build_test_prog.sh：

```bash
export ANDROID_NDK=你的ndk安装目录

rm -r build
mkdir build && cd build 

# CMake的内置支持
# cmake -DCMAKE_SYSTEM_NAME=Android \
# 	-DCMAKE_SYSTEM_VERSION=29 \
# 	-DCMAKE_ANDROID_ARCH_ABI=x86_64 \
# 	-DANDROID_NDK=$ANDROID_NDK \
# 	-DCMAKE_ANDROID_STL_TYPE=c++_shared \
# 	..

# 工具链文件支持
cmake \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=x86_64 \
    -DANDROID_PLATFORM=android-29 \
	-DANDROID_STL=c++_shared \
	  ..

cmake --build .
```

编译程序并上传模拟器：

```bash
# 编译
sh ./build_test_prog.sh
# 打开模拟器，流程略
# 上传可执行文件
adb push build/test /data/local/tmp
# 上传 STL 动态库
adb push /home/zzh0838/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/x86_64-linux-android/libc++_shared.so /data/local/tmp
# 进入到模拟器 shell
adb shell
# 执行程序
cd /data/local/tmp
# 加载驱动程序
insmod hello_drv.ko
export LD_LIBRARY_PATH=/data/local/tmp && ./test -w "nihao"
```

执行程序的结果如下所示：

![image.png](https://cdn.nlark.com/yuque/0/2022/png/2613680/1662732836883-08876742-8c01-443a-bd7c-296dfc43bcf1.png#clientId=u0cbf0354-2874-4&crop=0&crop=0&crop=1&crop=1&from=paste&height=49&id=udb108a53&margin=%5Bobject%20Object%5D&name=image.png&originHeight=74&originWidth=588&originalType=binary&ratio=1&rotation=0&showTitle=false&size=16286&status=done&style=none&taskId=ua20ea1f9-b6d4-463b-bb8d-7d54cb0145b&title=&width=392)

## 4.总结

- App 读写文件时，打开的文件在内核中是如何表示的？
- 驱动编写的流程？

## 源码

你可在我的 github 仓库 https://github.com/dducd/AndroidSourceLearn/tree/main/Demos/HelloDriver 中下载到示例代码。

## 参考资料

- 《嵌入式Linux应用开发完全手册 第二版》 韦东山
- 《Linux程序设计》 Neil Matthew      Richard Stones


## 关于我

- 我叫阿豪，目前定居成都
- 2012 年开始从事 Android 系统定制和应用开发相关的工作
- 2015 年毕业于国防科技大学，毕业后从事 Android 相关的开发和研究工作
- 2019年初开始创业，从事 Android 系统开发工作
- 如果你对 Android 系统源码感兴趣可以扫码添加我的微信，相互学习交流。

![27c7e38ee991b9d1fb42cb3bdf352a7.jpg](https://cdn.nlark.com/yuque/0/2022/jpeg/2613680/1662174041146-53015bfc-12f7-4023-9131-0a9e51fd00a2.jpeg#clientId=u0593d637-e239-4&crop=0&crop=0&crop=1&crop=1&from=drop&id=ud527bf55&margin=%5Bobject%20Object%5D&name=27c7e38ee991b9d1fb42cb3bdf352a7.jpg&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&size=42506&status=done&style=none&taskId=uf620381e-5767-4559-867e-093d91d3256&title=#crop=0&crop=0&crop=1&crop=1&id=qxLzV&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)
