# CPP宏函数的注意点

宏函数中，使用括号将每个变量括起

宏函数不是类型安全的，预处理器不会执行类型检查

宏函数如果有多行

* 反斜线代表该行未结束，会串接下一行。
* 而如果宏里有多过一个语句（statement），就需要用 do { /* *...** / } while(0) 包裹成单个语句，否则会有如下的问题：

```cpp
#define M() a(); b()
if (cond)
    M();
else
    c();

/* 预处理后 */

if (cond)
    a(); b();
else /* <- else 缺乏对应 if */
    c();
```

**只用 {} 也不行：**

```cpp
#define M() { a(); b(); }

/* 预处理后 */

if (cond)
    { a(); b(); }; /* 最后的分号代表 if 语句结束 */
else               /* else 缺乏对应 if */
    c();
```

用 do while 就行了：

```cpp
#define M() do { a(); b(); } while(0)

/* 预处理后 */

if (cond)
    do { a(); b(); } while(0);
else
    c();
```
