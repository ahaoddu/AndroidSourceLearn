# Binder之C++示例

Android 源码中提供了一系列的类来简化 binder 驱动的使用。使得我们的编写 binder 相关的程序得以简化，下面我们来看看具体的一个binder跨进程方法调用的简单demo应该怎么写?

示例代码在 https://github.com/ahaoddu/AndroidSourceLearn/tree/main/Demos/BinderCppDemo 可以下载到

## 1. 定义通信协议

IHelloService.h

```c
class IHelloService: public IInterface {

public:
    DECLARE_META_INTERFACE(HelloService);
	virtual void sayHello() = 0;
	virtual int sayHelloTo(const char *name) = 0;

};

//服务端
class BnHelloService: public BnInterface<IHelloService> {

public:
    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    void sayHello();
    int sayHelloTo(const char *name);
};

//客户端
class BpHelloService: public BpInterface<IHelloService> {
public:
    BpHelloService(const sp<IBinder>& impl);
    void sayHello();
    int sayHelloTo(const char *name);
};

}

```

## 2. 服务端协议实现

```cpp

#define LOG_TAG "HelloService"
#include <log/log.h>
#include "IHelloService.h"


namespace android {


status_t BnHelloService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {

    switch(code) {
        case HELLO_SVR_CMD_SAYHELLO:  {
            sayHello();
            reply->writeInt32(0);
            return NO_ERROR;
        } break;
   
        case HELLO_SVR_CMD_SAYHELLO_TO: {
            int32_t policy =  data.readInt32();
			String16 name16_tmp = data.readString16(); 

			String16 name16 = data.readString16();
			String8 name8(name16);

			int cnt = sayHelloTo(name8.string());

			reply->writeInt32(0); 
			reply->writeInt32(cnt);

            return NO_ERROR;
        } break;
  
        default:
            return BBinder::onTransact(code, data, reply, flags);
  
    }
}

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

## 客户端协议实现

```cpp
#include "IHelloService.h"

namespace android {

BpHelloService::BpHelloService(const sp<IBinder>& impl):BpInterface<IHelloService>(impl) {

}

void BpHelloService::sayHello() {
    Parcel data, reply;
    data.writeInt32(0);
    data.writeString16(String16("IHelloService"));
    remote()->transact(HELLO_SVR_CMD_SAYHELLO, data, &reply);
}

int BpHelloService::sayHelloTo(const char *name) {
    Parcel data, reply;
    int exception;

    data.writeInt32(0);
    data.writeString16(String16("IHelloService"));
    data.writeString16(String16(name));
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
