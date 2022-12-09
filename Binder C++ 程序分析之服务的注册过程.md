# Binder C++ 程序分析之服务的注册过程

[示例程序](https://github.com/ahaoddu/BinderCppDemo)服务端的实现如下：


## 主函数流程

```c++
int main(int argc, char const *argv[])
{   
    //完成驱动初始化
    sp<ProcessState> proc(ProcessState::self());
    //获得 ServiceManager Binder 代理类
    sp<IServiceManager> sm = defaultServiceManager();
    //添加服务
    sm->addService(String16("hello"), new BnHelloService());

    //开启线程池，接受远程调用数据
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
    
    return 0;
}

```

## ProcessState 完成初始化

`sp<ProcessState> proc(ProcessState::self());` 完成了 Binder 驱动的初始化，具体实现如下：

```c++
// /frameworks/native/libs/binder/Static.h
sp<ProcessState> gProcess;

// /frameworks/native/libs/binder/ProcessState.cpp
//ProcessState 是一个单例类
// const char* kDefaultDriver = "/dev/binder";
sp<ProcessState> ProcessState::self()
{
    Mutex::Autolock _l(gProcessMutex);
    if (gProcess != nullptr) {
        return gProcess;
    }
    //调用构造函数
    gProcess = new ProcessState(kDefaultDriver);
    return gProcess;
}

ProcessState::ProcessState(const char *driver)
    : mDriverName(String8(driver))
    , mDriverFD(open_driver(driver)) //1. 调用 open_dirver 完成初始化
    , mVMStart(MAP_FAILED)
    , mThreadCountLock(PTHREAD_MUTEX_INITIALIZER)
    , mThreadCountDecrement(PTHREAD_COND_INITIALIZER)
    , mExecutingThreadsCount(0)
    , mMaxThreads(DEFAULT_MAX_BINDER_THREADS)
    , mStarvationStartTimeMs(0)
    , mManagesContexts(false)
    , mBinderContextCheckFunc(nullptr)
    , mBinderContextUserData(nullptr)
    , mThreadPoolStarted(false)
    , mThreadPoolSeq(1)
    , mCallRestriction(CallRestriction::NONE)
{
    if (mDriverFD >= 0) {
        // mmap the binder, providing a chunk of virtual address space to receive transactions.
        //2. 调用 mmap 完成映射
        mVMStart = mmap(nullptr, BINDER_VM_SIZE, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, mDriverFD, 0);
        if (mVMStart == MAP_FAILED) {
            // *sigh*
            ALOGE("Using %s failed: unable to mmap transaction memory.\n", mDriverName.c_str());
            close(mDriverFD);
            mDriverFD = -1;
            mDriverName.clear();
        }
    }

    LOG_ALWAYS_FATAL_IF(mDriverFD < 0, "Binder driver could not be opened.  Terminating.");
}
```

ProcessState 在构造函数中，先后调用了 open_driver 和 mmap 来完成 binder 驱动的初始化，其中 open_driver 实现如下：

```c++
static int open_driver(const char *driver)
{
    //使用 open 打开 /dev/binder 驱动
    int fd = open(driver, O_RDWR | O_CLOEXEC);
    if (fd >= 0) {
        int vers = 0;
        //Binder 版本
        status_t result = ioctl(fd, BINDER_VERSION, &vers);
        if (result == -1) {
            ALOGE("Binder ioctl to obtain version failed: %s", strerror(errno));
            close(fd);
            fd = -1;
        }
        //Binder 协议版本
        if (result != 0 || vers != BINDER_CURRENT_PROTOCOL_VERSION) {
          ALOGE("Binder driver protocol(%d) does not match user space protocol(%d)! ioctl() return value: %d",
                vers, BINDER_CURRENT_PROTOCOL_VERSION, result);
            close(fd);
            fd = -1;
        }
        //设置线程数
        size_t maxThreads = DEFAULT_MAX_BINDER_THREADS;
        result = ioctl(fd, BINDER_SET_MAX_THREADS, &maxThreads);
        if (result == -1) {
            ALOGE("Binder ioctl to set max threads failed: %s", strerror(errno));
        }
    } else {
        ALOGW("Opening '%s' failed: %s\n", driver, strerror(errno));
    }
    return fd;
}
```

## 调用 defaultServiceManager 函数获取 ServiceManager Binder 代理类

接下来看看 `defaultServiceManager` 的实现：

```c++
sp<IServiceManager> defaultServiceManager()
{
    if (gDefaultServiceManager != nullptr) return gDefaultServiceManager;

    {
        AutoMutex _l(gDefaultServiceManagerLock);
        while (gDefaultServiceManager == nullptr) {
            gDefaultServiceManager = interface_cast<IServiceManager>(
                ProcessState::self()->getContextObject(nullptr));
            if (gDefaultServiceManager == nullptr)
                sleep(1);
        }
    }

    return gDefaultServiceManager;
}
```

接下来逐步分析：`ProcessState::self()->getContextObject(nullptr)` :

```c++
sp<IBinder> ProcessState::getContextObject(const sp<IBinder>& /*caller*/)
{
    return getStrongProxyForHandle(0);
}


sp<IBinder> ProcessState::getStrongProxyForHandle(int32_t handle)
{
    sp<IBinder> result;

    AutoMutex _l(mLock);

    //1
    handle_entry* e = lookupHandleLocked(handle);

    //2
    if (e != nullptr) {
        IBinder* b = e->binder;
        if (b == nullptr || !e->refs->attemptIncWeak(this)) {
            if (handle == 0) {
                Parcel data;
                status_t status = IPCThreadState::self()->transact(
                        0, IBinder::PING_TRANSACTION, data, nullptr, 0);
                if (status == DEAD_OBJECT)
                   return nullptr;
            }
            //3
            b = BpBinder::create(handle);
            e->binder = b;
            if (b) e->refs = b->getWeakRefs();
            result = b;
        } else {
            result.force_set(b);
            e->refs->decWeak(this);
        }
    }

    return result;
}

Vector<handle_entry> mHandleToObject;

ProcessState::handle_entry* ProcessState::lookupHandleLocked(int32_t handle)
{
    const size_t N=mHandleToObject.size();
    if (N <= (size_t)handle) {
        handle_entry e;
        e.binder = nullptr;
        e.refs = nullptr;
        status_t err = mHandleToObject.insertAt(e, N, handle+1-N);
        if (err < NO_ERROR) return nullptr;
    }
    return &mHandleToObject.editItemAt(handle);
}
```

1. lookupHandleLocked()，是在 `Vector mHandleToObject` (这里不是 c++ 标准库中的 Vector，具体实现在 `/system/core/libutils/include/utils/KeyedVector.h` 中)中查找是否有句柄为 handle 的handle_entry 对象。有的话，则返回该 handle_entry 对象；没有的话，则新建 handle 对应的 handle_entry，并将其添加到 mHandleToObject 中，然后再返回。mHandleToObject 是用于保存各个 IBinder 代理对象的 Vector 数组，它相当于一个缓冲。

2. 很显然，此时 e! = NULL 为 true，进入 if(e!=NULL) 中。而此时 e->binder=NULL，并且 handle=0；则调 IPCThreadState::self()->transact() 尝试去和 Binder 驱动通信(尝试去ping内核中Binder驱动)。由于 Binder 驱动已启动，ping通信是能够成功的。

3. 接着，调用 BpBinder::create(handle)（其内部实际是 new 一个 BpBinder 对象），并赋值给 e->binder。然后，将该 BpBinder 对象返回。



从上面的分析知道 `ProcessState::self()->getContextObject(nullptr)` 返回了一个 BpBinder 对象，其内部的 mHandle 值为 0。

接下来调用 `interface_cast<IServiceManager>` 宏，将 BpBinder 转换为 IServiceManager

下面看下 `interface_cast<IServiceManager>`

```c++
template<typename INTERFACE>
inline sp<INTERFACE> interface_cast(const sp<IBinder>& obj)
{
    return INTERFACE::asInterface(obj);
}

//模板展开
inline sp<IServiceManager> interface_cast(const sp<IBinder>& obj)
{
    return IServiceManager::asInterface(obj);
}
```

asInterface 是 IServiceManager 类的一个静态方法，通过 IMPLEMENT_META_INTERFACE 宏实现。宏展开后如下：

```c++
::android::sp<IServiceManager> IServiceManager::asInterface(              
            const ::android::sp<::android::IBinder>& obj)               
    {                                                                   
        ::android::sp<IServiceManager> intr;                               
        if (obj != nullptr) {                                           
            intr = static_cast<IServiceManager*>(    
                //BpBinder 的 queryLocalInterface 函数返回 null                     
                obj->queryLocalInterface(                               
                        IServiceManager::descriptor).get());               
            if (intr == nullptr) {    //走这里 
                //obj 类型是 BpBinder                                 
                intr = new BpServiceManager(obj);                          
            }                                                           
        }                                                               
        return intr;                                                    
    }               
```

asInterface  函数实际是 new 了一个 BpServiceManager，并传入了 obj，obj 的类型是 BpBinder。

回到开始处，我们把新构建的 BpServiceManager 赋值给了全局变量 gDefaultServiceManager，后面我们就可以通过这个代理类发起远程调用。


## 发起远程调用

```c++
sm->addService(String16("hello"), new BnHelloService());
```

sm 的实际类型是 BpServiceManager ，我们通过这个 Binder 代理类发起远程调用。
BnHelloService 是 Hello 服务对应的 Binder 本地类。


接下来看看 addService 的具体实现：

```c++
    //声明
    //后两个参数带默认值
    virtual status_t addService(const String16& name, const sp<IBinder>& service,
                                bool allowIsolated = false,
                                int dumpsysFlags = DUMP_FLAG_PRIORITY_DEFAULT) = 0;
    //实现
    virtual status_t addService(const String16& name, const sp<IBinder>& service,
                                bool allowIsolated, int dumpsysPriority) {
        Parcel data, reply;
        //写入头数据
        data.writeInterfaceToken(IServiceManager::getInterfaceDescriptor());
        data.writeString16(name);
        //将 BnHelloService 封装到 flat_binder_object 结构体中
        data.writeStrongBinder(service);
        data.writeInt32(allowIsolated ? 1 : 0);
        data.writeInt32(dumpsysPriority);
        status_t err = remote()->transact(ADD_SERVICE_TRANSACTION, data, &reply);
        return err == NO_ERROR ? reply.readExceptionCode() : err;
    }
```




