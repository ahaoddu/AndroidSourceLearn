# Binder C++ 程序示例

Android 源码中提供了一系列的类来简化 Binder 驱动的使用。使得开发者能快速在 Android 系统源码中添加 Binder 服务和使用 Binder 服务。接下来我们使用 Android 源码中提供的辅助类和 C++ 语言来编写一个 Binder C++ 示例程序。

可以在[这里](https://github.com/ahaoddu/BinderCppDemo)下载到。

使用 C++ 实现一个 Binder 服务分为以下几步：
* 定义协议
* 实现服务端协议
* 实现客户端协议
* 服务端程序实现
* 客户端程序实现 

## 1. 定义通信协议

通信协议由三部分组成：
* 通信协议接口
* 通信协议服务端实现
* 通信协议客户端实现

接下来我们来实现通信协议接口 IHelloService， 接口继承自 IInterface，描述了名为 Hello 的 Binder 服务对外提供的功能。

```c++
//IHelloService.h
class IHelloService: public IInterface {

public:
    //DECLARE_META_INTERFACE 是一个宏，声明了一些变量和函数
    DECLARE_META_INTERFACE(HelloService);
    //Binder 服务对外提供的功能
    virtual void sayHello() = 0;
    virtual int sayHelloTo(const char *name) = 0;

};
```

## 2. 通信协议服务端实现

通信协议服务端实现是一个继承自 `BnInterface<IHelloService>` 的类：

```c++
//声明
//IHelloService.h
class BnHelloService: public BnInterface<IHelloService> {

public:
    //服务端收到数据的回调
    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    void sayHello();
    int sayHelloTo(const char *name);
};


//实现
//BnHelloService.cpp
#define LOG_TAG "HelloService"
#include <log/log.h>
#include "IHelloService.h"

namespace android {

//服务端收到数据的回调
status_t BnHelloService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    //code 表示客户端需要调用哪个函数
    switch(code) {
        //调用 sayhello
        case HELLO_SVR_CMD_SAYHELLO:  {
            //调用 sayhello 函数
            sayHello();
            //写入回复给客户端的数据
            reply->writeInt32(0);
            return NO_ERROR;
        } break;
        //调用 sayhelloto
        case HELLO_SVR_CMD_SAYHELLO_TO: {
            //取出客户端发送的参数
            int32_t policy =  data.readInt32();
			String16 name16_tmp = data.readString16(); 

			String16 name16 = data.readString16();
			String8 name8(name16);
            //调用 sayHelloTo 函数
			int cnt = sayHelloTo(name8.string());
            //写入回复给客户端的数据
			reply->writeInt32(0); 
			reply->writeInt32(cnt);

            return NO_ERROR;
        } break;
  
        default:
            return BBinder::onTransact(code, data, reply, flags);
  
    }
}

//服务端函数的具体实现
void BnHelloService::sayHello() {
    static int count = 0;
    ALOGI("say hello :%d\n ", ++count);
}

int BnHelloService::sayHelloTo(const char *name) {
    static int cnt = 0;
	ALOGI("say hello to %s : %d\n", name, ++cnt);
	return cnt;
}

}
```

为叙述方便我们称 BnHelloService 以及同类型的类为 `Binder 服务类`

## 3. 通信协议客户端协议实现

客户端协议实现是一个继承自 BpInterface<IHelloService> 的类

```cpp
//客户端
//IHelloService.h
class BpHelloService: public BpInterface<IHelloService> {
public:
    BpHelloService(const sp<IBinder>& impl);
    void sayHello();
    int sayHelloTo(const char *name);
};


//BpHelloService.cpp
#include "IHelloService.h"

namespace android {

BpHelloService::BpHelloService(const sp<IBinder>& impl):BpInterface<IHelloService>(impl) {

}

void BpHelloService::sayHello() {
    Parcel data, reply;
    data.writeInt32(0);
    data.writeString16(String16("IHelloService"));
    //发起远程调用
    remote()->transact(HELLO_SVR_CMD_SAYHELLO, data, &reply);
}

int BpHelloService::sayHelloTo(const char *name) {
    Parcel data, reply;
    int exception;

    data.writeInt32(0);
    data.writeString16(String16("IHelloService"));
    data.writeString16(String16(name));
    //发起远程调用
    remote()->transact(HELLO_SVR_CMD_SAYHELLO_TO, data, &reply);
    exception = reply.readInt32();
	if (exception)
		return -1;
	else
		return reply.readInt32();
}

    IMPLEMENT_META_INTERFACE(HelloService, "android.media.IHelloService");
}
```

为叙述方便我们称 BpHelloService 以及同类型的类为 `Binder 代理类`

## 4. 服务端程序实现

```c++
int main(int argc, char const *argv[])
{
    //使用 ProcessState 类完成 binder 驱动的初始化
    sp<ProcessState> proc(ProcessState::self());
    //注册服务
    sp<IServiceManager> sm = defaultServiceManager();
    sm->addService(String16("hello"), new BnHelloService());

    //开启 binder 线程池
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
    
    return 0;
}
```

## 5. 客户端程序实现

```c++
int main(int argc, char const *argv[])
{
    //使用 ProcessState 类完成 binder 驱动的初始化
    sp<ProcessState> proc(ProcessState::self());
    //获取 hello 服务
    sp<IServiceManager> sm = defaultServiceManager();
    //返回的是 BpBinder 指针
    sp<IBinder> binder = sm->getService(String16("hello"));
    sp<IHelloService> service =
		    interface_cast<IHelloService>(binder);

    if (binder == 0)
	{
		ALOGI("can't get hello service\n");
		return -1;
	}
    //发起远程调用
    service->sayHello();
    int cnt = service->sayHelloTo("nihao");
	ALOGI("client call sayhello_to, cnt = %d", cnt);


    return 0;
}
```

## 6. 编译运行

和 binder c 程序的编译运行过程一致，可以参考 [Binder C 程序示例](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/Binder%20C%20%E7%A8%8B%E5%BA%8F%E7%A4%BA%E4%BE%8B.md)第四节。