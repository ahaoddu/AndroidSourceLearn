# 预备知识-Android C++ 层基石BaseRef wp sp

## 引子


## LightRefBase

```cpp
template <class T>
class LightRefBase
{
public:
    inline LightRefBase() : mCount(0) { }
    inline void incStrong(__attribute__((unused)) const void* id) const {
        mCount.fetch_add(1, std::memory_order_relaxed);
    }
    inline void decStrong(__attribute__((unused)) const void* id) const {
        if (mCount.fetch_sub(1, std::memory_order_release) == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete static_cast<const T*>(this);
        }
    }
    //! DEBUGGING ONLY: Get current strong ref count.
    inline int32_t getStrongCount() const {
        return mCount.load(std::memory_order_relaxed);
    }

    typedef LightRefBase<T> basetype;

protected:
    inline ~LightRefBase() { }

private:
    friend class ReferenceMover;
    inline static void renameRefs(size_t /*n*/, const ReferenceRenamer& /*renamer*/) { }
    inline static void renameRefId(T* /*ref*/, const void* /*old_id*/ , const void* /*new_id*/) { }

private:
    mutable std::atomic<int32_t> mCount;
};
```


## 参考资料

* [Android系统的智能指针（轻量级指针、强指针和弱指针）的实现原理分析](https://blog.csdn.net/Luoshengyang/article/details/6786239)
* 《深入理解Android》 卷一 第五章
