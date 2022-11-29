# Binder C 程序分析

回顾流程：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/ab78aa31091d94cf922c8a5ef82a8c2.jpg)

* ServiceManager 完成在 Binder 驱动的注册，等待接受其他进程的请求
* Server 向 ServiceManager 注册服务
* Client 向 ServiceManger 查询服务，使用服务，即通过 Binder 驱动调用定义在 Server 端的方法

## binder_open

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

    bs->fd = open(driver, O_RDWR | O_CLOEXEC); //打开 /dev/binder，拿到内核返回的句柄
    if (bs->fd < 0) {
        fprintf(stderr,"binder: cannot open %s (%s)\n",
                driver, strerror(errno));
        goto fail_open;
    }

    //版本验证
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