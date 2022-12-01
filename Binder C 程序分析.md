# Binder C 程序分析

我们先回顾一下Binder 的工作流程：

* 进程 B 将他的一堆函数（称为服务）注册到 ServiceManager 中
* 进程 A 向 ServiceManager 查询到 B 后，获得一个句柄 handle
* 进程 A 通过这个 handle 访问到 B 中定义的函数。


具体的 C 程序流程如下图所示：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/ab78aa31091d94cf922c8a5ef82a8c2.jpg)


* ipc rpc 概念
* 引出 我们要研究数据结构
* 介绍 API open ioctl
* 情景分析1：注册服务
* 情景分析2：获取服务
* 情景分析3：调用服务

## 1. 理解 IPC 与 RPC 概念

* IPC：（Inter Process Communication ）跨进程通信：泛指进程之间的数据交换。
* RPC：（Reomote Procedure Call） 远程过程调用：指在进程 A 中可以调用到进程 B 中的函数，RPC 的一般基于 IPC 实现。

## 应用层数据格式分析

在之前的 [C 程序示例中](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/Binder%20C%20%E7%A8%8B%E5%BA%8F%E7%A4%BA%E4%BE%8B.md)，我们实现了 client 调用 server 中 hello 函数的功能。过程如下图所示：

对于应用程序来说，我们最关心的是：

* client 发起调用时，client 发给驱动的数据的格式是什么？
* server 收到数据是，驱动 发送个人 server 的数据格式是什么？
* servicemanager 收到的数据是什么格式？内部保存了哪些数据？

在分析数据格式之前我们先了解几个 Binder 相关的 API：

* open()，用于打开 binder 驱动，返回 Binder 驱动的文件描述符
* mmap()，用于在内核中申请一块内存，并完成应用层与内核层的虚拟地址映射
* ioctl，在应用层调用 ioctl 向内核层发送数据或者接受内核层发送到应用层的数据，其参数结构如下：

```c
ioctl(文件描述符,ioctl命令,数据)
```

文件描述符是在调用 open 时的返回值，ioctl 命令和第三个参数"数据"的类型是相关联的，具体如下：

| ioctl命令   |      数据类型      |  函数动作 |
|:----------:|:-------------:|:------:|
| BINDER_WRITE_READ |  struct binder_write_read | 应用层向内核层收发数据 |
| BINDER_SET_MAX_THREADS |    size_t   | 设置最大线程数 |
| BINDER_SET_CONTEXT_MGR | int |   设置当前进程 |
| BINDER_THREAD_EXIT | int | 删除 binder 线程 |
| BINDER_VERSION |  struct binder_version | 获取 binder 协议版本 |

接下来我们从具体的代码情景中来分析我们关心的数据结构：


服务的注册过程：

//todo 流程图

1-3 ：ServiceManager 调用 open() 函数，打开 binder 驱动，而后调用 mmap 完成内存的申请与映射，通过 ioctl 注册为 contextmanager，再调用 ioctl 函数等待接受数据，并进入待机状态。

对应的代码如下：

```c
//省略部分非核心代码
int main(int argc, char** argv)
{
    struct binder_state *bs;
    union selinux_callback cb;
    char *driver;

    if (argc > 1) {
        driver = argv[1];
    } else {
        driver = "/dev/binder";
    }
    //调用 open 和 mmap
    bs = binder_open(driver, 128*1024);
    if (!bs) {
#ifdef VENDORSERVICEMANAGER
        ALOGW("failed to open binder driver %s\n", driver);
        while (true) {
            sleep(UINT_MAX);
        }
#else
        ALOGE("failed to open binder driver %s\n", driver);
#endif
        return -1;
    }

    if (binder_become_context_manager(bs)) {
        ALOGE("cannot become context manager (%s)\n", strerror(errno));
        return -1;
    }

    binder_loop(bs, svcmgr_handler);

    return 0;
}

```






##  注册服务过程分析

```c
int main(int argc, char **argv)
{
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
	int ret;
    
    //打开驱动
    bs = binder_open("/dev/binder", 128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

	//添加服务
	ret = svcmgr_publish(bs, svcmgr, "hello", hello_service_handler);
    if (ret) {
        fprintf(stderr, "failed to publish hello service\n");
        return -1;
    } 
    
    //进入循环
    binder_loop(bs, test_server_handler);
    return 0;
}
```



##  ServiceManager 实现分析

源码路径：`frameworks/native/cmds/servicemanager/service_manager.c`

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

    //初始化 binder 驱动
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

ServiceManager 工作流程简单明了：

* 通过 binder_open 函数初始化 binder 驱动
* 通过 binder_become_context_manager 将当前进程注册为 context manager
* 调用 binder_loop 进入循环，等待远程调用

接下来我们逐个分析关键函数：

### binder_open 函数分析

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


1.2 binder_become_context_manager

`binder_become_context_manager` 的作用是让 ServiceManager 成为整个系统中唯一的上下文管理器，其实也就是 service 管理器。具体实现如下：

```c
int binder_become_context_manager(struct binder_state *bs)
{
    //构建需要发送的数据 flat_binder_object
    struct flat_binder_object obj;
    memset(&obj, 0, sizeof(obj));
    obj.flags = FLAT_BINDER_FLAG_TXN_SECURITY_CTX;

    //向 Binder 驱动发送数据
    int result = ioctl(bs->fd, BINDER_SET_CONTEXT_MGR_EXT, &obj);

    //如果失，使用原始方式再次调用 ioctl
    // fallback to original method
    if (result != 0) {
        android_errorWriteLog(0x534e4554, "121035042");

        result = ioctl(bs->fd, BINDER_SET_CONTEXT_MGR, 0);
    }
    return result;
}
```

主要流程如下：

* 构建需要发送的数据 flat_binder_object
* 通过 ioctl 将构造好的数据发送给 Binder 驱动
* 如果失败，使用原始方式再次调用 ioctl

其中核心的方法就是 ioctl ，ioctl 是 linux 驱动框架中的一个函数，用于从应用中向内核驱动发送数据，具体的功能和实现在 binder 驱动实现分析中来讲解。

### binder_loop

binder_loop 用于进入循环，等待远程调用。

调用方式如下：

```c
binder_loop(bs, svcmgr_handler);
```

其中 svcmgr_handler 是一个函数指针，是收到远程调用后的回调，其实现如下：

```c
//txn_secctx msg 是调用方发来的数据
//reply 是返回给调用方的数据
int svcmgr_handler(struct binder_state *bs,
                   struct binder_transaction_data_secctx *txn_secctx,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    //......
}
```

当 client 查询服务或者 server 注册服务的时候，会向 service_manager 发送请求，service_manager 收到后，就会回调 svcmgr_handler 函数。  

收到的数据格式包括了 binder_io， binder_transaction_data_secctx

binder_io 可以理解为一个数据集合，发送方按照固定的顺序存数据，接收方按照相同的顺序将数据取出。

binder_transaction_data_secctx 是一个简单的封装，我们需要的数据都保存在 binder_transaction_data 中：

```c
struct binder_transaction_data_secctx {
	struct binder_transaction_data transaction_data;
	binder_uintptr_t secctx;
};

struct binder_transaction_data {
	union {
		__u32	handle;
		binder_uintptr_t ptr;
	} target;
	binder_uintptr_t	cookie;	
	__u32		code;

	__u32	        flags;
	pid_t		sender_pid;
	uid_t		sender_euid;
	binder_size_t	data_size;	
	binder_size_t	offsets_size;	

	union {
		struct {
			binder_uintptr_t	buffer;
			binder_uintptr_t	offsets;
		} ptr;
		__u8	buf[8];
	} data;
};
```

binder_transaction_data 内部包含了很多数据，直接解释其作用会显得苍白无力，我们在分析代码的时候再来解释其作用。

接下来分析 svcmgr_handler 的实现：

```c
// bs 是 binder_open 中构建的结构体
//txn_secctx 和 msg 是收到的数据
//reply 用于回复数据
int svcmgr_handler(struct binder_state *bs,
                   struct binder_transaction_data_secctx *txn_secctx,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    //一堆变量，暂时不管
    struct svcinfo *si;
    uint16_t *s;
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
    int allow_isolated;
    uint32_t dumpsys_priority;

    //获取到 binder_transaction_data 结构体数据
    struct binder_transaction_data *txn = &txn_secctx->transaction_data;

    //对收到的数据做一些检查，暂时不管
    if (txn->target.ptr != BINDER_SERVICE_MANAGER)
        return -1;

    if (txn->code == PING_TRANSACTION)
        return 0;

    // msg 中第一个数据一般是一个 32 位的 0
    strict_policy = bio_get_uint32(msg);
    // msg 中的第二个数据 ，一般也是 32 位的 0
    bio_get_uint32(msg); 
    // msg 中的第三个数据，一个字符串，正常情况是 android.os.IServiceManager
    s = bio_get_string16(msg, &len);
    if (s == NULL) {
        return -1;
    }

    // svcmgr_id 是一个字符数组，内容是 android.os.IServiceManager
    if ((len != (sizeof(svcmgr_id) / 2)) ||
        memcmp(svcmgr_id, s, sizeof(svcmgr_id))) {
        fprintf(stderr,"invalid id %s\n", str8(s, len));
        return -1;
    }

    //selinux 相关代码 省略 ......

    //code 代表需要调用哪个函数
    switch(txn->code) {
    //获取服务
    case SVC_MGR_GET_SERVICE:
    case SVC_MGR_CHECK_SERVICE:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }
        handle = do_find_service(s, len, txn->sender_euid, txn->sender_pid,
                                 (const char*) txn_secctx->secctx);
        if (!handle)
            break;
        bio_put_ref(reply, handle);
        return 0;
    //添加服务
    case SVC_MGR_ADD_SERVICE:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }
        handle = bio_get_ref(msg);
        allow_isolated = bio_get_uint32(msg) ? 1 : 0;
        dumpsys_priority = bio_get_uint32(msg);
        if (do_add_service(bs, s, len, handle, txn->sender_euid, allow_isolated, dumpsys_priority,
                           txn->sender_pid, (const char*) txn_secctx->secctx))
            return -1;
        break;
    // list 已注册的服务
    case SVC_MGR_LIST_SERVICES: {
        uint32_t n = bio_get_uint32(msg);
        uint32_t req_dumpsys_priority = bio_get_uint32(msg);

        if (!svc_can_list(txn->sender_pid, (const char*) txn_secctx->secctx, txn->sender_euid)) {
            ALOGE("list_service() uid=%d - PERMISSION DENIED\n",
                    txn->sender_euid);
            return -1;
        }
        si = svclist;
        // walk through the list of services n times skipping services that
        // do not support the requested priority
        while (si) {
            if (si->dumpsys_priority & req_dumpsys_priority) {
                if (n == 0) break;
                n--;
            }
            si = si->next;
        }
        if (si) {
            bio_put_string16(reply, si->name);
            return 0;
        }
        return -1;
    }
    default:
        ALOGE("unknown code %d\n", txn->code);
        return -1;
    }

    bio_put_uint32(reply, 0);
    return 0;
}
```

接下来我们来分析一下添加服务相关的代码：

```c
    //添加服务
    case SVC_MGR_ADD_SERVICE:
        //获取到服务的名字
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }
        //服务在内核中的句柄
        handle = bio_get_ref(msg);
        allow_isolated = bio_get_uint32(msg) ? 1 : 0;
        dumpsys_priority = bio_get_uint32(msg);
        if (do_add_service(bs, s, len, handle, txn->sender_euid, allow_isolated, dumpsys_priority,
                           txn->sender_pid, (const char*) txn_secctx->secctx))
            return -1;
        break;
```

do_add_service 的实现如下：

```c
int do_add_service(struct binder_state *bs, const uint16_t *s, size_t len, uint32_t handle,
                   uid_t uid, int allow_isolated, uint32_t dumpsys_priority, pid_t spid, const char* sid) {
    struct svcinfo *si;

    if (!handle || (len == 0) || (len > 127))
        return -1;

    //一些权限判断，暂时可以不管
    if (!svc_can_register(s, len, spid, sid, uid)) {
        ALOGE("add_service('%s',%x) uid=%d - PERMISSION DENIED\n",
             str8(s, len), handle, uid);
        return -1;
    }

    //源码中定义了一个单项链表用于存 service
    // 链表的节点是 svcinfo，链表头是 svclist 全局变量
    //这里从链表中查找服务，这里是注册服务，链表中没有当前服务，查找到的值是 null
    si = find_svc(s, len);
    if (si) { // si 为 null
        if (si->handle) {
            ALOGE("add_service('%s',%x) uid=%d - ALREADY REGISTERED, OVERRIDE\n",
                 str8(s, len), handle, uid);
            svcinfo_death(bs, si);
        }
        si->handle = handle;
    } else { // 代码走这里
        //构建新的节点
        si = malloc(sizeof(*si) + (len + 1) * sizeof(uint16_t));
        if (!si) {
            ALOGE("add_service('%s',%x) uid=%d - OUT OF MEMORY\n",
                 str8(s, len), handle, uid);
            return -1;
        }
        // 保存数据，并加入链表
        si->handle = handle;
        si->len = len;
        memcpy(si->name, s, (len + 1) * sizeof(uint16_t));
        si->name[len] = '\0';
        si->death.func = (void*) svcinfo_death;
        si->death.ptr = si;
        si->allow_isolated = allow_isolated;
        si->dumpsys_priority = dumpsys_priority;
        si->next = svclist;
        svclist = si;
    }
    //binder_ref强引用加1操作，留到内核部分讲解，这里暂时不管
    binder_acquire(bs, handle);
    //注册死亡通知，留到内核部分讲解，这里暂时不管
    binder_link_to_death(bs, handle, &si->death);
    return 0;
}
```

总结一下就是：

* 构建 svcinfo 链表节点
* 根据收到的数据，给 svcinfo 赋值
* 将 svcinfo 添加到 svclist 链表中

svcinfo 中最重要的数据是 handle，用于标识一个 service，其值由驱动确定，并发送给了 servicemanger，保存在 svclist 链表中。

接下来再回头来看 binder_loop 具体实现：

```c
void binder_loop(struct binder_state *bs, binder_handler func)
{
    int res;
    //ioctl 读写数据类型
    struct binder_write_read bwr;
    uint32_t readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    //告诉驱动，应用程序要进入循环了
    readbuf[0] = BC_ENTER_LOOPER;
    //ioctl 的基本封装
    binder_write(bs, readbuf, sizeof(uint32_t));

    for (;;) {
        //结合上面 bwr 的赋值，这里是要读数据
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (uintptr_t) readbuf;
        //向驱动发起读操作
        res = ioctl(bs->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }

        //解析收到的数据，func 是解析好数据后的回调函数
        res = binder_parse(bs, 0, (uintptr_t) readbuf, bwr.read_consumed, func);
        if (res == 0) {
            ALOGE("binder_loop: unexpected reply?!\n");
            break;
        }
        if (res < 0) {
            ALOGE("binder_loop: io error %d %s\n", res, strerror(errno));
            break;
        }
    }
}
```






## 参考资料

* 《Android 框架解密》
* [Android10.0 Binder通信原理(三)-ServiceManager篇](https://blog.csdn.net/yiranfeng/article/details/105210069)
* 韦东山 Binder 视频教程