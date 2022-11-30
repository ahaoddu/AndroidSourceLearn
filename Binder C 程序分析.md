# Binder C 程序分析

回顾流程：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/ab78aa31091d94cf922c8a5ef82a8c2.jpg)

* ServiceManager 完成在 Binder 驱动的注册，等待接受其他进程的请求
* Server 向 ServiceManager 注册服务
* Client 向 ServiceManger 查询服务，使用服务，即通过 Binder 驱动调用定义在 Server 端的方法


## ServiceManager 启动过程分析

源码路径：frameworks/native/cmds/servicemanager/service_manager.c

看下主函数：

```c
int main(int argc, char** argv)
{
    struct binder_state *bs;
    char *driver;

    if (argc > 1) {
        driver = argv[1];
    } else {
        driver = "/dev/binder";
    }

    //打开驱动
    bs = binder_open(driver, 128*1024);
    if (!bs) {
        //省略 VENDORSERVICEMANAGER 相关代码 ......
        ALOGE("failed to open binder driver %s\n", driver);
        return -1;
    }

    //注册当前进程为 context_manager
    if (binder_become_context_manager(bs)) {
        ALOGE("cannot become context manager (%s)\n", strerror(errno));
        return -1;
    }

    // 省略 selinux 相关代码 ......

    //进入循环，等待远程调用
    //svcmgr_handler 是一个函数指针，是远程调用的回调函数
    binder_loop(bs, svcmgr_handler);

    return 0;
}
```



binder_open 用于初始化 binder 驱动。

binder_open 的调用过程如下：
```c    
bs = binder_open("/dev/binder", 128*1024);
```

binder_open 的实现如下：

```c
// driver 通常是 "/dev/binder"
// mapsize 是需要 mmap 的内存的大小
struct binder_state *binder_open(const char* driver, size_t mapsize)
{
    struct binder_state *bs; //用于存需要返回的值
    struct binder_version vers; 

    bs = malloc(sizeof(*bs)); 
    if (!bs) {
        errno = ENOMEM;
        return NULL;
    }

    //打开 /dev/binder，拿到内核返回的句柄
    bs->fd = open(driver, O_RDWR | O_CLOEXEC); 
    if (bs->fd < 0) {
        fprintf(stderr,"binder: cannot open %s (%s)\n",
                driver, strerror(errno));
        goto fail_open;
    }

    //查询版本
    if ((ioctl(bs->fd, BINDER_VERSION, &vers) == -1) ||
        (vers.protocol_version != BINDER_CURRENT_PROTOCOL_VERSION)) {
        fprintf(stderr,
                "binder: kernel driver version (%d) differs from user space version (%d)\n",
                vers.protocol_version, BINDER_CURRENT_PROTOCOL_VERSION);
        goto fail_open;
    }

    //完成内存映射
    bs->mapsize = mapsize;
    bs->mapped = mmap(NULL, mapsize, PROT_READ, MAP_PRIVATE, bs->fd, 0);
    if (bs->mapped == MAP_FAILED) {
        fprintf(stderr,"binder: cannot map device (%s)\n",
                strerror(errno));
        goto fail_map;
    }

    return bs;

fail_map:
    close(bs->fd);
fail_open:
    free(bs);
    return NULL;
}
```

其中 `struct binder_state *bs` 结构如下：

```cpp
struct binder_state
{
    int fd;
    void *mapped;
    size_t mapsize;
};
```
用于保存 binder_open 的返回结果。

binder_open()的工作也比较简单，分为以下几步：

1. 通过系统调用 open() 来打开 "/dev/binder"，获得一个文件句柄信息。
2. 通过 ioctl 获取 binder 的版本信息，比较 binder 协议版本是否相同，不同则跳出。
3. 通过 mmap 内存映射 128K 的内存空间，即把 binder 驱动文件的 128K 字节映射到了内存空间。

## 参考资料
* [Android10.0 Binder通信原理(三)-ServiceManager篇](https://blog.csdn.net/yiranfeng/article/details/105210069)