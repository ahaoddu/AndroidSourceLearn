#include <linux/module.h>
#include <linux/init.h>

/**
 * __init 是一个宏，表示 hello_init 是一个初始化函数，会放到编译后目标文件的初始化段中
 */ 
static int __init hello_init(void)
{
	//printk 是内核中的日志打印函数
	printk("Hello world!\n");
	return 0;
}
 
/**
 * __exit 是一个宏，表示 hello_exit 是一个初始化函数，会放到编译后目标文件的初始化段中
 */ 
static void __exit hello_exit(void)
{
	printk("hello exit\n");
}

/**
 * hello_init 是当前模块的启动函数
 */ 
module_init(hello_init);
/*
 * hello_exit 是当前模块的退出函数
 */
module_exit(hello_exit);