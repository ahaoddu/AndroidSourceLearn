# Binder C++ 程序分析之主要类解析

## 1. 引子

在 C 程序中，服务就是函数的集合。C++ 中将服务封装为了一堆 Bn Bp 开头的类。

这些类可以分为以下几类：

* 协议类：即 Server 端提供了哪些函数供 Client 端调用。在示例程序中，协议类就是 IHelloService
* Server 端类：这些类一般 Bn 开头，意思应该是 Binder native。包括了 BBinder，BnInterface，BnHelloService
* Client 端类：这些类一般 Bp 开头，意思应该是 Binder proxy。包括了 BpBider，BpRefBase，BpInterface，BpHelloService


接下来，我们一一分析这一堆的类：

## 2. 协议类 IHelloService

首先我们看看协议类 IHelloService:

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

IHelloService 是我们定义的服务协议类，该协议类同时描述了：
* Server 端提供了 sayHello sayHelloto 函数供 Client 端调用
* Client 端可以通过 sayHello sayHelloto 发起远程调用

IHelloService 继承自 IInterface， 其实现如下：

```c++
//IInterface.h
class IInterface : public virtual RefBase
{
public:
            IInterface();
            static sp<IBinder>  asBinder(const IInterface*);
            static sp<IBinder>  asBinder(const sp<IInterface>&);

protected:
    virtual                     ~IInterface();
    virtual IBinder*            onAsBinder() = 0;
};

//IInerface.cpp

// static
sp<IBinder> IInterface::asBinder(const IInterface* iface)
{
    if (iface == nullptr) return nullptr;
    return const_cast<IInterface*>(iface)->onAsBinder();
}

// static
sp<IBinder> IInterface::asBinder(const sp<IInterface>& iface)
{
    if (iface == nullptr) return nullptr;
    return iface->onAsBinder();
}
```
IInterface 接口中最重要的方法是 asBinder，用于将一个服务接口转化为 IBinder。在 asBinder 中会调用 onAsBinder 来完成具体的转换，onAsBinder 是一个虚函数，由子类实现。至于怎么转和为什么转，我们后面遇到再说。


IHelloService 的定义中我们使用了一个宏 DECLARE_META_INTERFACE ，其定义如下：
```c++
#define DECLARE_META_INTERFACE(INTERFACE)                               \
public:                                                                 \
    static const ::android::String16 descriptor;                        \
    static ::android::sp<I##INTERFACE> asInterface(                     \
            const ::android::sp<::android::IBinder>& obj);              \
    virtual const ::android::String16& getInterfaceDescriptor() const;  \
    I##INTERFACE();                                                     \
    virtual ~I##INTERFACE();                                            \
    static bool setDefaultImpl(std::unique_ptr<I##INTERFACE> impl);     \
    static const std::unique_ptr<I##INTERFACE>& getDefaultImpl();       \
private:                                                                \
    static std::unique_ptr<I##INTERFACE> default_impl;                  \
public:  
```

将宏展开后，IHelloService 定义如下：

```c++
class IHelloService: public IInterface {
public:                                                                 
    static const ::android::String16 descriptor;                        
    static ::android::sp<IHelloService> asInterface(const ::android::sp<::android::IBinder>& obj);              
    virtual const ::android::String16& getInterfaceDescriptor() const;  
    IHelloService();                                                     
    virtual ~IHelloService();                                            
    static bool setDefaultImpl(std::unique_ptr<IHelloService> impl);     
    static const std::unique_ptr<IHelloService>& getDefaultImpl();       
private:                                                                
    static std::unique_ptr<IHelloService> default_impl;                  
public:  
    //Binder 服务对外提供的功能
    virtual void sayHello() = 0;
    virtual int sayHelloTo(const char *name) = 0;
};
```

展开的宏中，最重要的函数是 asInterface，用于将一个 IBinder 对象转换为 IHelloService 对象。至于怎么转和为什么转，我们后面遇到再说。

对应的类图如下:

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221208161550.png)


## 3. Server 端相关类

Server 端对协议类 IHelloService 实现为 BnHelloService：

```c++
//声明
//IHelloService.h
class BnHelloService: public BnInterface<IHelloService> {

public:
    //服务端收到数据时，会回调该函数
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
//这里只是简单的打一些 log
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

服务端的实现继承自 BnInterface<IHelloService>，IHelloService 作为泛型传入

BnInterface 定义如下：

```c++
template<typename INTERFACE>
class BnInterface : public INTERFACE, public BBinder
{
public:
    virtual sp<IInterface>      queryLocalInterface(const String16& _descriptor);
    virtual const String16&     getInterfaceDescriptor() const;

protected:
    typedef INTERFACE           BaseInterface;
    virtual IBinder*            onAsBinder();
};
```

泛型展开后如下：

```c++
template<typename IHelloService>
class BnInterface : public IHelloService, public BBinder
{
public:
    virtual sp<IInterface>      queryLocalInterface(const String16& _descriptor);
    virtual const String16&     getInterfaceDescriptor() const;

protected:
    typedef INTERFACE           BaseInterface;
    virtual IBinder*            onAsBinder();
};
```

可以看出 BnInterface 继承自 IHelloService 和 BBinder，

BBinder 更准确的名字应该是 BnBinder，其具体实现如下：

```cpp
class BBinder : public IBinder
{
public:
                        BBinder();

    virtual const String16& getInterfaceDescriptor() const;
    virtual bool        isBinderAlive() const;
    virtual status_t    pingBinder();
    virtual status_t    dump(int fd, const Vector<String16>& args);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    transact(   uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    linkToDeath(const sp<DeathRecipient>& recipient,
                                    void* cookie = nullptr,
                                    uint32_t flags = 0);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    unlinkToDeath(  const wp<DeathRecipient>& recipient,
                                        void* cookie = nullptr,
                                        uint32_t flags = 0,
                                        wp<DeathRecipient>* outRecipient = nullptr);

    virtual void        attachObject(   const void* objectID,
                                        void* object,
                                        void* cleanupCookie,
                                        object_cleanup_func func);
    virtual void*       findObject(const void* objectID) const;
    virtual void        detachObject(const void* objectID);

    virtual BBinder*    localBinder();

    bool                isRequestingSid();
    // This must be called before the object is sent to another process. Not thread safe.
    void                setRequestingSid(bool requestSid);

protected:
    virtual             ~BBinder();

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);

private:
                        BBinder(const BBinder& o);
            BBinder&    operator=(const BBinder& o);

    class Extras;

    Extras*             getOrCreateExtras();

    std::atomic<Extras*> mExtras;
            void*       mReserved0;
};
```

其中最重要的方法是 onTransact，作为在收到远程调用后的回调，我们在子类 BnHelloService 中完成了具体的实现。

BBinder 类继承自 IBinder，IBinder 是 Binder 相关操作的抽象，其实现如下：
```c++
class [[clang::lto_visibility_public]] IBinder : public virtual RefBase
{
public:
    enum {
        FIRST_CALL_TRANSACTION  = 0x00000001,
        LAST_CALL_TRANSACTION   = 0x00ffffff,

        PING_TRANSACTION        = B_PACK_CHARS('_','P','N','G'),
        DUMP_TRANSACTION        = B_PACK_CHARS('_','D','M','P'),
        SHELL_COMMAND_TRANSACTION = B_PACK_CHARS('_','C','M','D'),
        INTERFACE_TRANSACTION   = B_PACK_CHARS('_', 'N', 'T', 'F'),
        SYSPROPS_TRANSACTION    = B_PACK_CHARS('_', 'S', 'P', 'R'),

        // Corresponds to TF_ONE_WAY -- an asynchronous call.
        FLAG_ONEWAY             = 0x00000001
    };

                          IBinder();

    /**
     * Check if this IBinder implements the interface named by
     * @a descriptor.  If it does, the base pointer to it is returned,
     * which you can safely static_cast<> to the concrete C++ interface.
     */
    virtual sp<IInterface>  queryLocalInterface(const String16& descriptor);

    /**
     * Return the canonical name of the interface provided by this IBinder
     * object.
     */
    virtual const String16& getInterfaceDescriptor() const = 0;

    virtual bool            isBinderAlive() const = 0;
    virtual status_t        pingBinder() = 0;
    virtual status_t        dump(int fd, const Vector<String16>& args) = 0;
    static  status_t        shellCommand(const sp<IBinder>& target, int in, int out, int err,
                                         Vector<String16>& args, const sp<IShellCallback>& callback,
                                         const sp<IResultReceiver>& resultReceiver);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t        transact(   uint32_t code,
                                        const Parcel& data,
                                        Parcel* reply,
                                        uint32_t flags = 0) = 0;

    // DeathRecipient is pure abstract, there is no virtual method
    // implementation to put in a translation unit in order to silence the
    // weak vtables warning.
    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
    #endif

    class DeathRecipient : public virtual RefBase
    {
    public:
        virtual void binderDied(const wp<IBinder>& who) = 0;
    };

    #if defined(__clang__)
    #pragma clang diagnostic pop
    #endif

    /**
     * Register the @a recipient for a notification if this binder
     * goes away.  If this binder object unexpectedly goes away
     * (typically because its hosting process has been killed),
     * then DeathRecipient::binderDied() will be called with a reference
     * to this.
     *
     * The @a cookie is optional -- if non-NULL, it should be a
     * memory address that you own (that is, you know it is unique).
     *
     * @note You will only receive death notifications for remote binders,
     * as local binders by definition can't die without you dying as well.
     * Trying to use this function on a local binder will result in an
     * INVALID_OPERATION code being returned and nothing happening.
     *
     * @note This link always holds a weak reference to its recipient.
     *
     * @note You will only receive a weak reference to the dead
     * binder.  You should not try to promote this to a strong reference.
     * (Nor should you need to, as there is nothing useful you can
     * directly do with it now that it has passed on.)
     */
    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t        linkToDeath(const sp<DeathRecipient>& recipient,
                                        void* cookie = nullptr,
                                        uint32_t flags = 0) = 0;

    /**
     * Remove a previously registered death notification.
     * The @a recipient will no longer be called if this object
     * dies.  The @a cookie is optional.  If non-NULL, you can
     * supply a NULL @a recipient, and the recipient previously
     * added with that cookie will be unlinked.
     *
     * If the binder is dead, this will return DEAD_OBJECT. Deleting
     * the object will also unlink all death recipients.
     */
    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t        unlinkToDeath(  const wp<DeathRecipient>& recipient,
                                            void* cookie = nullptr,
                                            uint32_t flags = 0,
                                            wp<DeathRecipient>* outRecipient = nullptr) = 0;

    virtual bool            checkSubclass(const void* subclassID) const;

    typedef void (*object_cleanup_func)(const void* id, void* obj, void* cleanupCookie);

    /**
     * This object is attached for the lifetime of this binder object. When
     * this binder object is destructed, the cleanup function of all attached
     * objects are invoked with their respective objectID, object, and
     * cleanupCookie. Access to these APIs can be made from multiple threads,
     * but calls from different threads are allowed to be interleaved.
     */
    virtual void            attachObject(   const void* objectID,
                                            void* object,
                                            void* cleanupCookie,
                                            object_cleanup_func func) = 0;
    /**
     * Returns object attached with attachObject.
     */
    virtual void*           findObject(const void* objectID) const = 0;
    /**
     * WARNING: this API does not call the cleanup function for legacy reasons.
     * It also does not return void* for legacy reasons. If you need to detach
     * an object and destroy it, there are two options:
     * - if you can, don't call detachObject and instead wait for the destructor
     *   to clean it up.
     * - manually retrieve and destruct the object (if multiple of your threads
     *   are accessing these APIs, you must guarantee that attachObject isn't
     *   called after findObject and before detachObject is called).
     */
    virtual void            detachObject(const void* objectID) = 0;

    virtual BBinder*        localBinder();
    virtual BpBinder*       remoteBinder();

protected:
    virtual          ~IBinder();

private:
};
```

Server 端的类图总结如下：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221208174649.png)



## 4. Client 端相关类

Client 端对协议类 IHelloService 实现为 BnHelloService：

```c++
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

//包装数据，并发起远程调用
void BpHelloService::sayHello() {
    Parcel data, reply;
    data.writeInt32(0);
    data.writeString16(String16("IHelloService"));
    //发起远程调用
    remote()->transact(HELLO_SVR_CMD_SAYHELLO, data, &reply);
}

//包装数据，并发起远程调用
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

BpHelloService 继承自 BpInterface<IHelloService>,BpInterface 是一个模板类：

```c++
class BpInterface : public IHelloService, public BpRefBase
{
public:
    explicit                    BpInterface(const sp<IBinder>& remote);
protected:
    typedef INTERFACE           BaseInterface;
    virtual IBinder*            onAsBinder();
};
```

泛型展开后如下：

```c++
class BpInterface : public IHelloService, public BpRefBase
{
public:
    explicit                    BpInterface(const sp<IBinder>& remote);
protected:
    typedef IHelloService           BaseInterface;
    virtual IBinder*            onAsBinder();
};
```

BpInterface 继承自 IHelloService 和 BpRefBase，BpRefBase 的实现如下：


```c++
class BpRefBase : public virtual RefBase
{
protected:
    explicit                BpRefBase(const sp<IBinder>& o);
    virtual                 ~BpRefBase();
    virtual void            onFirstRef();
    virtual void            onLastStrongRef(const void* id);
    virtual bool            onIncStrongAttempted(uint32_t flags, const void* id);

    inline  IBinder*        remote()                { return mRemote; }
    inline  IBinder*        remote() const          { return mRemote; }

private:
                            BpRefBase(const BpRefBase& o);
    BpRefBase&              operator=(const BpRefBase& o);

    //核心，指向一个 BpBinder 类型
    IBinder* const          mRemote;
    RefBase::weakref_type*  mRefs;
    std::atomic<int32_t>    mState;
};
```

BpBinder 实现如下：

```cpp
class BpBinder : public IBinder
{
public:
    static BpBinder*    create(int32_t handle);

    inline  int32_t     handle() const { return mHandle; }

    virtual const String16&    getInterfaceDescriptor() const;
    virtual bool        isBinderAlive() const;
    virtual status_t    pingBinder();
    virtual status_t    dump(int fd, const Vector<String16>& args);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    transact(   uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    linkToDeath(const sp<DeathRecipient>& recipient,
                                    void* cookie = nullptr,
                                    uint32_t flags = 0);

    // NOLINTNEXTLINE(google-default-arguments)
    virtual status_t    unlinkToDeath(  const wp<DeathRecipient>& recipient,
                                        void* cookie = nullptr,
                                        uint32_t flags = 0,
                                        wp<DeathRecipient>* outRecipient = nullptr);

    virtual void        attachObject(   const void* objectID,
                                        void* object,
                                        void* cleanupCookie,
                                        object_cleanup_func func);
    virtual void*       findObject(const void* objectID) const;
    virtual void        detachObject(const void* objectID);

    virtual BpBinder*   remoteBinder();

            status_t    setConstantData(const void* data, size_t size);
            void        sendObituary();

    static uint32_t     getBinderProxyCount(uint32_t uid);
    static void         getCountByUid(Vector<uint32_t>& uids, Vector<uint32_t>& counts);
    static void         enableCountByUid();
    static void         disableCountByUid();
    static void         setCountByUidEnabled(bool enable);
    static void         setLimitCallback(binder_proxy_limit_callback cb);
    static void         setBinderProxyCountWatermarks(int high, int low);

    class ObjectManager
    {
    public:
                    ObjectManager();
                    ~ObjectManager();

        void        attach( const void* objectID,
                            void* object,
                            void* cleanupCookie,
                            IBinder::object_cleanup_func func);
        void*       find(const void* objectID) const;
        void        detach(const void* objectID);

        void        kill();

    private:
                    ObjectManager(const ObjectManager&);
        ObjectManager& operator=(const ObjectManager&);

        struct entry_t
        {
            void* object;
            void* cleanupCookie;
            IBinder::object_cleanup_func func;
        };

        KeyedVector<const void*, entry_t> mObjects;
    };

protected:
                        BpBinder(int32_t handle,int32_t trackedUid);
    virtual             ~BpBinder();
    virtual void        onFirstRef();
    virtual void        onLastStrongRef(const void* id);
    virtual bool        onIncStrongAttempted(uint32_t flags, const void* id);

private:
    const   int32_t             mHandle;

    struct Obituary {
        wp<DeathRecipient> recipient;
        void* cookie;
        uint32_t flags;
    };

            void                reportOneDeath(const Obituary& obit);
            bool                isDescriptorCached() const;

    mutable Mutex               mLock;
            volatile int32_t    mAlive;
            volatile int32_t    mObitsSent;
            Vector<Obituary>*   mObituaries;
            ObjectManager       mObjects;
            Parcel*             mConstantData;
    mutable String16            mDescriptorCache;
            int32_t             mTrackedUid;

    static Mutex                                sTrackingLock;
    static std::unordered_map<int32_t,uint32_t> sTrackingMap;
    static int                                  sNumTrackedUids;
    static std::atomic_bool                     sCountByUidEnabled;
    static binder_proxy_limit_callback          sLimitCallback;
    static uint32_t                             sBinderProxyCountHighWatermark;
    static uint32_t                             sBinderProxyCountLowWatermark;
    static bool                                 sBinderProxyThrottleCreate;
};
```

类图总结如下：
![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221209162846.png)


## 5. 总结

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221208184420.png)
