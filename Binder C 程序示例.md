# Binder C 程序示例

## 1. 引子

无论是应用开发还是系统开发，编程人员接触到的一般是 Java 层和 C++ 的封装库。这些库的使用相对简单的，这是因为 Android 团队在 C++ 层和  Java 层封装了大量的辅助类简化了应用层的开发。直接学习它们容易陷入在这些库的设计和封装上，不能接触到 Binder 最原本的样子。为了更好地学习 Binder，我们需要去搜寻 Binder 最原始的使用方式。

Binder 在内核中注册为杂项设备，实际是一个字符驱动，了解 linux 驱动的话，我们应该使用 write read open ioctl  mmap 等函数访问字符驱动。

源码中搜寻一番后，在 `frameworks/native/cmds/servicemanager` 目录下终于找到了 C 语言完成的 Binder 程序。看下目录下的文件：

```bash
.
├── Android.bp
├── bctest.c
├── binder.c
├── binder.h
├── service_manager.c
├── servicemanager.rc
└── vndservicemanager.rc
```

其中的 bctest 是一个 C语言完成的 Binder 客户端程序。

在掉了一把头发后，我发现源码中的示例程序是不完整的。于是，在掉了更多头发以后，我找到了一个[相对完整的 Binder C 层示例程序](https://github.com/weidongshan/APP_0003_Binder_C_App)，进行了部分修改后，在 Android 10 下顺利运行起来了。下面我们一步一步写出这个示例程序。

建议在[这里](https://github.com/ahaoddu/BinderCDemo)将源码下载到本地后，再继续后面的分析和学习。


写代码之前我们需要明白：

Binder 是一个 RPC（Remote Procedure Call） 框架，简单说就是使用 Binder，我们可以在 A 进程中访问 B 进程中定义的函数。

另外，Binder 中，A 是不能直接访问到 B 的，还需要一个第三者 ServiceManager，B 将他的一堆函数（称为服务）注册到 ServiceManager 中，A 向 ServiceManager 查询到 B 后，获得一个句柄 handle（一个int值，用于标记B注册的服务），通过这个 handle 就可以访问到 B 中定义的函数了。

整个程序涉及的对象：

* server：服务端，定义服务，服务就是一堆函数。
* client：客户端，访问服务，即访问 server 中定义的函数
* servicemamager：用于管理服务
* binder驱动：提供跨进程的数据传输功能即 IPC

其工作流程如下图所示：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/ab78aa31091d94cf922c8a5ef82a8c2.jpg)


简单归纳一下：
* ServiceManager 完成在 Binder 驱动的注册，等待接受其他进程的请求
* Server 向 ServiceManager 注册服务，进入循环等待远程访问调用
* Client 向 ServiceManger 查询服务，使用服务，即通过 Binder 驱动调用定义在 Server 端的方法

## 2. Server 端程序编写

编写 Server 端程序的主要流程如下：

* 定义 hello service
* 通过 open 函数初始化 binder 驱动
* 向 ServiceManager 注册服务
* 进入 loop， 等待 client 请求服务


首先我们需要定义 server 端的 service，service 就是一些函数：

```c++
//hello 服务提供的函数1
void sayhello(void)
{
	static int cnt = 0;
	fprintf(stderr, "say hello : %d\n", ++cnt);
}

//hello 服务提供的函数2
int sayhello_to(char *name)
{
	static int cnt = 0;
	fprintf(stderr, "say hello to %s : %d\n", name, ++cnt);
	return cnt;
}


// server 收到 client 远程函数调用后的回调函数，用于处理收到的信息
// bs 表示 binder 状态
// txn_secctx 和 msg 是收到的数据
// binder_io 是 c 层定义的数据结构，提供了保存数据和取出数据的函数，方便数据的传输
// reply 是需要返回给 client 的数据
int hello_service_handler(struct binder_state *bs,
                   struct binder_transaction_data_secctx *txn_secctx,
                   struct binder_io *msg,
                   struct binder_io *reply)
{

    struct binder_transaction_data *txn = &txn_secctx->transaction_data;

	/* 根据txn->code知道要调用哪一个函数
	 * 如果需要参数, 可以从msg取出
	 * 如果要返回结果, 可以把结果放入reply
	 */

	/* sayhello
	 * sayhello_to
	 */
	
    uint16_t *s;
    char name[512];
    size_t len;
    //uint32_t handle;
    uint32_t strict_policy;
    int i;

    strict_policy = bio_get_uint32(msg);

    switch(txn->code) {
        //调用函数1
        case HELLO_SVR_CMD_SAYHELLO:
            sayhello();
            bio_put_uint32(reply, 0); /* no exception */
            return 0;
        //调用函数2
        case HELLO_SVR_CMD_SAYHELLO_TO:
        /* 从msg里取出字符串 */
            s = bio_get_string16(msg, &len);  //"IHelloService"
            s = bio_get_string16(msg, &len);  // name
            if (s == NULL) {
	            return -1;
            }
            for (i = 0; i < len; i++)
                name[i] = s[i];

            name[i] = '\0';

            /* 处理 */
            i = sayhello_to(name);

            /* 把结果放入reply */
            bio_put_uint32(reply, 0); /* no exception */
            bio_put_uint32(reply, i);
            break;

        default:
            fprintf(stderr, "unknown code %d\n", txn->code);
            return -1;
    }

        return 0;
}
```

定义好 service 后，我们就可以开始写 server 的 main 函数了：

```c++
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
    
    //进入无限循环，等待 client 的请求到来
    //test_server_handler 是收到数据后的回调函数
    binder_loop(bs, test_server_handler);
    return 0;
}
```

## 3. Client 端程序编写

编写 Client 程序的主要流程如下：

* 实现远程调用函数
* open 初始化 binder 驱动
* 查询服务，获取到服务的句柄 handle
* 通过 handle 调用远程调用函数


首先定义好远程调用函数：

```c
void sayhello(void)
{
    unsigned iodata[512/4];
    //binder_io 用于存取数据
    struct binder_io msg, reply;

	/* 构造binder_io */
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IHelloService");

	/* 放入参数 */

	/* 调用 binder_call 发起远程调用 */
    if (binder_call(g_bs, &msg, &reply, g_handle, HELLO_SVR_CMD_SAYHELLO))
        return ;
	
	/* 从reply中解析出返回值 */

    binder_done(g_bs, &msg, &reply);
	
}

int sayhello_to(char *name)
{
	unsigned iodata[512/4];
	struct binder_io msg, reply;
	int ret;
	int exception;

	/* 构造binder_io */
	bio_init(&msg, iodata, sizeof(iodata), 4);
	bio_put_uint32(&msg, 0);  // strict mode header
        bio_put_string16_x(&msg, "IHelloService");

	/* 放入参数 */
        bio_put_string16_x(&msg, name);

	/* 调用binder_call */
	if (binder_call(g_bs, &msg, &reply, g_handle, HELLO_SVR_CMD_SAYHELLO_TO))
		return 0;
	
	/* 从reply中解析出返回值 */
	exception = bio_get_uint32(&reply);
	if (exception)
		ret = -1;
	else
		ret = bio_get_uint32(&reply);

	binder_done(g_bs, &msg, &reply);

	return ret;
	
}
```

接下来，实现 main 函数：

```c
int g_handle = 0;
struct binder_state *g_bs;

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
	int ret;

    //初始化 binder 驱动
    bs = binder_open("/dev/binder", 128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    g_bs = bs;

	//查找服务，获取到服务的句柄 handle
    g_handle = svcmgr_lookup(bs, svcmgr, "hello");
    if (!g_handle) {
        ALOGW("binder client 查找服务 hello 失败");
        return -1;
    } else {
        ALOGW("binder client 查找服务成功 handle = %d", g_handle);
    }

    //通过 handle 调用服务
    sayhello();
    sayhelloto("hello binder");
```

最后我们来编写 Android.bp

```bash
cc_defaults {
    name: "bindertestflags",


    cflags: [
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unused-parameter",
        "-Wno-missing-field-initializers",
        "-Wno-unused-parameter",
        "-Wno-unused-variable",
        "-Wno-incompatible-pointer-types",
        "-Wno-sign-compare",
    ],
    product_variables: {
        binder32bit: {
            cflags: ["-DBINDER_IPC_32BIT=1"],
        },
    },

    shared_libs: ["liblog"],
}

cc_binary {
    name: "binderclient",
    defaults: ["bindertestflags"],
    srcs: [
        "client.cpp",
        "binder.c",
    ],
}

cc_binary {
    name: "binderserver",
    defaults: ["bindertestflags"],
    srcs: [
        "server.cpp",
        "binder.c",
    ],
}
```


## 4. 编译运行程序

将我们的源码拷贝到 aosp 源码目录下。

在 aosp 目录下执行以下命令:

```bash
source build/envsetup.sh
lunch aosp_x86_64-eng
```

进入 BinderCDemo，编译模块：

```bash
cd BinderCDemo
mm
```

编译完成后，将程序上传到模拟器并执行。

传输可执行文件到模拟器：

```bash
adb push aosp/out/target/product/my_generic_x86_64/vendor/bin/binderserver /data/local/tmp

adb push aosp/out/target/product/my_generic_x86_64/vendor/bin/binderclient /data/local/tmp
```

接下来 `adb shell` 进入模拟器 `shell` 环境：

```bash
adb shell
cd /data/local/tmp
./binderserver
# 从新开一个终端进入 adb shell
cd /data/local/tmp
./binderclient
```

最后通过 logcat 查看执行结果：

```bash
logcat | grep "BinderServer"

11-22 17:08:39.133  6594  6594 W BinderServer: say hello : 1
```


