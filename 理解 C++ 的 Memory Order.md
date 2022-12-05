# 理解 C++ 的 Memory Order

## 1. 为什么需要 Memory Order

如果不使用任何同步机制（例如 mutex 或 atomic），在多线程中读写同一个变量，那么，程序的结果是难以预料的。主要原因有一下几点：

* 简单的读写不是原子操作
* CPU 可能会调整指令的执行顺序
* 在 CPU cache 的影响下，一个 CPU 执行了某个指令，不会立即被其它 CPU 看见

### 1.1 非原子操作给多线程编程带来的影响

原子操作说的是，一个操作的状态要么就是未执行，要么就是已完成，不会看见中间状态。

下面看一个非原子操作给多线程编程带来的影响：

```cpp
  	int64_t i = 0;     // global variable
Thread-1:              Thread-2:
i++;               std::cout << i;
```

C++ 并不保证 `i++` 是原子操作。从汇编的角度看，读写内存的操作一般分为三步：

* 将内存单元读到 cpu 寄存器
* 修改寄存器中的值
* 将寄存器中的值回写入对应的内存单元

进一步，有的 CPU Architecture， 64 位数据（int64_t）在内存和寄存器之间的读写需要两条指令。

这就导致了 i++ 操作在 cpu 的角度是一个多步骤的操作。所以 Thread-2 读到的可能是一个中间状态。

### 1.2 指令的执行顺序调整给多线程编程带来的影响

为了优化程序的执行性能，编译器和 CPU 可能会调整指令的执行顺序。为阐述这一点，下面的例子中，让我们假设所有操作都是原子操作：

```cpp
  	  int x = 0;     // global variable
          int y = 0;     // global variable
		  
Thread-1:              Thread-2:
x = 100;               while (y != 200) {}
y = 200;               std::cout << x;
   
```

如果 CPU 没有乱序执行指令，那么 Thread-2 将输出 `100`。然而，对于 Thread-1 来说，`x = 100;` 和 `y = 200;` 这两个语句之间没有依赖关系，因此，Thread-1 允许调整语句的执行顺序：

```cpp
Thread-1:
y = 200;
x = 100;
```

在这种情况下，Thread-2 将输出 `0` 或 `100`。

### 1.3 CPU CACHE 对多线程程序的影响

CPU cache 也会影响到程序的行为。下面的例子中，假设从时间上来讲，A 操作先于 B 操作发生：

```cpp
 	    int x = 0;     // global variable
		  
Thread-1:                      Thread-2:
x = 100;    // A               std::cout << x;    // B

```

尽管从时间上来讲，A 先于 B，但 CPU cache 的影响下，Thread-2 不能保证立即看到 A 操作的结果，所以 Thread-2 可能输出 `0` 或 `100`。

## 2. 同步机制

对于 C++ 程序来说，解决以上问题的办法就是使用同步机制，最常见的同步机制就是 `std::mutex `和 `std::atomic`。从性能角度看，通常使用 `std::atomic` 会获得更好的性能。

C++ 提供了四种 memory ordering ：

* Relaxed ordering
* Release-Acquire ordering
* Release-Consume ordering
* Sequentially-consistent ordering

### 2.1 Relaxed ordering

在这种模型下，`std::atomic` 的 `load()` 和 `store()` 都要带上 `memory_order_relaxed` 参数。Relaxed ordering 仅仅保证 `load()` 和 `store()` 是原子操作，除此之外，不提供任何跨线程的同步。


先看看一个简单的例子：

```cpp
                   std::atomic<int> x = 0;     // global variable
                   std::atomic<int> y = 0;     // global variable
		  
Thread-1:                              Thread-2:
//A                                    // C
r1 = y.load(memory_order_relaxed);     r2 = x.load(memory_order_relaxed); 
//B                                    // D
x.store(r1, memory_order_relaxed);     y.store(42, memory_order_relaxed); 

```



执行完上面的程序，可能出现  `r1 == r2 == 42`。理解这一点并不难，因为编译器允许调整 C 和 D 的执行顺序。如果程序的执行顺序是 D -> A -> B -> C，那么就会出现 `r1 == r2 == 42`。

如果某个操作只要求是原子操作，除此之外，不需要其它同步的保障，就可以使用 Relaxed ordering。程序计数器是一种典型的应用场景：

```cpp
#include <cassert>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
std::atomic<int> cnt = {0};
void f()
{
    for (int n = 0; n < 1000; ++n) {
        cnt.fetch_add(1, std::memory_order_relaxed);
    }
}
int main()
{
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f);
    }
    for (auto& t : v) {
        t.join();
    }
    assert(cnt == 10000);    // never failed
    return 0;
}
```

### 2.2 Release-Acquire ordering

在这种模型下，`store()` 使用 `memory_order_release`，而 `load()` 使用 `memory_order_acquire`。这种模型有两种效果，第一种是可以限制 CPU 指令的重排：

* 在 `store()` 之前的所有读写操作，不允许被移动到这个 `store()` 的后面。
* 在 `load()` 之后的所有读写操作，不允许被移动到这个 `load()` 的前面。

除此之外，还有另一种效果：假设 Thread-1 `store()` 的那个值，成功被 Thread-2 `load()` 到了，那么 Thread-1 在 `store()` 之前对内存的所有写入操作，此时对 Thread-2 来说，都是可见的。


下面的例子阐述了这种模型的原理：

```cpp
#include <thread>
#include <atomic>
#include <cassert>
#include <string>

std::atomic<bool> ready{ false };
int data = 0;
void producer()
{
    data = 100;                                       // A
    ready.store(true, std::memory_order_release);     // B
}
void consumer()
{
    while (!ready.load(std::memory_order_acquire)){}    // C
    assert(data == 100); // never failed              // D
}
int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}
```



让我们分析一下这个过程：

* 首先 A 不允许被移动到 B 的后面。
* 同样 D 也不允许被移动到 C 的前面。
* 当 C 从 while 循环中退出了，说明 C 读取到了 B `store() `的那个值，此时，Thread-2 保证能够看见 Thread-1 执行 B 之前的所有写入操作（也即是 A）。


### 2.3 Release-Consume ordering

在这种模型下，`store()` 使用 `memory_order_release`，而 `load()` 使用 ` memory_order_consume`。这种模型有两种效果，第一种是可以限制 CPU 指令的重排：

* 在 `store()` 之前的与原子变量相关的所有读写操作，不允许被移动到这个 `store()` 的后面。
* 在 `load()` 之后的与原子变量相关的所有读写操作，不允许被移动到这个 `load()` 的前面。

除此之外，还有另一种效果：假设 Thread-1 `store()` 的那个值，成功被 Thread-2 `load()` 到了，那么 Thread-1 在 `store()` 之前对与原子变量相关的内存的所有写入操作，此时对 Thread-2 来说，都是可见的。

下面的例子阐述了这种模型的原理：

```cpp
  #include <thread>
#include <atomic>
#include <cassert>
#include <string>
 
std::atomic<std::string*> ptr;
int data;
 
void producer()
{
    std::string* p  = new std::string("Hello");  //A
    data = 42;
    //ptr依赖于p
    ptr.store(p, std::memory_order_release);   //B
}
 
void consumer()
{
    std::string* p2;
    while (!(p2 = ptr.load(std::memory_order_consume))) //C
        ;
    // never fires: *p2 carries dependency from ptr
    assert(*p2 == "Hello");                           //D
    // may or may not fire: data does not carry dependency from ptr
    assert(data == 42); 
}
 
int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join(); t2.join();
}
```


让我们分析一下这个过程：

* 首先 A 不允许被移动到 B 的后面。
* 同样 D 也不允许被移动到 C 的前面。
* data 与 ptr 无关，不会限制他的重排序
* 当 C 从 while 循环中退出了，说明 C 读取到了 B `store() `的那个值，此时，Thread-2 保证能够看见 Thread-1 执行 B 之前的与原子变量相关的所有写入操作（也即是 A）。

### 2.4 Sequentially-consistent ordering

Sequentially-consistent ordering 是缺省设置，在 Release-Acquire ordering 限制的基础上，保证了所有设置了 `memory_order_seq_cst` 标志的原子操作按照代码的先后顺序执行。


## 参考资料

* [理解 C++ 的 Memory Order](http://senlinzhan.github.io/2017/12/04/cpp-memory-order/)
* 《C++新经典》 第十七章
* 《C++并发编程实战（第2版）》第五章
* [如何理解 C++11 的六种 memory order？](https://www.zhihu.com/question/24301047)
* [现代C++的内存模型](https://zhuanlan.zhihu.com/p/382372072)
* [C++ atomics and memory ordering](https://bartoszmilewski.com/2008/12/01/c-atomics-and-memory-ordering/)
* [Understanding Atomics and Memory Ordering](https://dev.to/kprotty/understanding-atomics-and-memory-ordering-2mom)
* [atomic Weapons: The C++ Memory Model and Modern Hardware](https://herbsutter.com/2013/02/11/atomic-weapons-the-c-memory-model-and-modern-hardware/)
* [C++11新特性内存模型总结详解--一篇秒懂](https://www.cnblogs.com/bclshuai/p/15898116.html)
* [C++ Memory_order的理解](https://blog.csdn.net/baidu_20351223/article/details/115765606)
* [memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order#Release-Acquire_ordering)
