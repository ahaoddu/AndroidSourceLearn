# 添加自定义模块之 C&CPP 库

## 1. Android.mk 方式集成

在 `aosp/device/mycompamy/product/` 目录下创建以下的目录和文件：

```bash
.
├── AndroidProducts.mk
├── libmath
│   ├── Android.mk
│   ├── my_math.c
│   └── my_math.h
├── libmath2
│   ├── Android.mk
│   ├── my_math2.c
│   └── my_math2.h
├── main
│   ├── Android.mk
│   └── main.c
└── myaosp.mk
```

其中 AndroidProducts.mk myaosp.mk 是 [配置product](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/blob/main/4.Framework%E5%BC%80%E5%8F%91/%E9%85%8D%E7%BD%AEProduct.md) 中添加的 product 配置文件。

### 1.1 libmath 动态库

libmath 是一个 C 语言动态库。其 Android.mk 内容如下：

```makefile
# 固定内容
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# 源码位置
LOCAL_SRC_FILES:= \
        my_math.c

# 模块名
LOCAL_MODULE:= libmymathmk

# 编译为 64 为库
LOCAL_MULTILIB := 64

LOCAL_MODULE_TAGS := optional

# 编译到vender
LOCAL_VENDOR_MODULE := true

# 表示这是一个动态库
include $(BUILD_SHARED_LIBRARY)

```

my_math.h 内容如下：

```cpp
#ifndef __MY_MATH_H__
#define __MY_MATH_H__

int my_add(int a, int b);
int my_sub(int a, int b);

#endif
```

my_math.c 内容如下：

```cpp
#include "my_math.h"

int my_add(int a, int b)
{
	return a + b;
}

int my_sub(int a, int b)
{
	return a - b;
}

```

接下来我们就可以在 libmath 目录下执行 mm 命令，编译当前模块：

```cpp
mm
......
device/mycompamy/product/myaosp.mk was modified, regenerating...
out/soong/Android-myaosp.mk was modified, regenerating...
[100% 15/15] build out/target/product/my_generic_x86_64/obj/SHARED_LIBRARIES/libmymathmk_intermediates/lib

#### build completed successfully (01:16 (mm:ss)) ####
```



### 1.2 libmath2 静态库

libmath2 目录下：

Android.mk:

```makefile

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    my_math2.c \

LOCAL_MODULE:= libmymath2mk
LOCAL_MULTILIB := 64

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true

# 静态库
include $(BUILD_STATIC_LIBRARY)

```

my_math2.c：

```cpp
#include "my_math2.h"

int my_multi(int a, int b)
{
	return a * b;
}

int my_div(int a, int b)
{
	return a / b;
}

```

my_math2.h：

```cpp
#ifndef __MY_MATH2_H__
#define __MY_MATH2_H__

int my_multi(int a, int b);

int my_div(int a, int b);

#endif
```

同样，通过 mm 命令编译。

### 1.3 构建可执行程序并引用两个库

main 目录下：

main.c

```cpp
#include <stdio.h>
#include "my_math.h"
#include "my_math2.h"

int main(int argc, char *argv[])
{
	printf("a + b = %d\n", my_add(30, 40));
	printf("a * b = %d\n", my_multi(30, 40));

	return 0;
}
```

Android.bp：

```makefile

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		main.c

# 指定头文件
LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/../libmath \
		$(LOCAL_PATH)/../libmath2


LOCAL_SHARED_LIBRARIES += \
	libmymathmk 

LOCAL_STATIC_LIBRARIES += \
	libmymath2mk 

LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS += \
		-Wno-error \
		-Wno-unused-parameter

LOCAL_MODULE:= mymathtestmk 

LOCAL_MODULE_TAGS := optional


LOCAL_MULTILIB := 64


include $(BUILD_EXECUTABLE)



```

在 `aosp/device/mycompamy/product/myaosp.mk` 中添加：

```bash
PRODUCT_PACKAGES += mymathtestmk
```

接下来编译系统：

```bash
source build/envsetup.sh
lunch myaosp-eng
make -j16
```

编译完成启动虚拟机后，就可以通过 adb shell 运行我们的 hello 程序了

```bash
adb shell mymathtestmk
```

执行结果如下图所示：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221012170126.png)


## 2. Android.bp 方式集成

将上文的 libmath/Android.mk 修改为 Android.bp：

```bash
cc_library_shared {
    name: "libmymathbp",

    srcs: ["my_math.c"],

    export_include_dirs: ["."],

    vendor: true,

    compile_multilib: "64",

}

```

libmath2/Android.mk 修改为 Android.bp：

```makefile

cc_library_static {
    name: "libmymath2bp",

    srcs: ["my_math2.c"],

    export_include_dirs: ["."],

    compile_multilib: "64",
}

```

将 main/Android.mk 修改为 Android.bp

```makefile
cc_binary {
    name: "mymathtestbp",

    srcs: ["main.c"],

    shared_libs: ["libmymath"],

    static_libs: ["libmycjson"],
    cflags: ["-Wno-error"] + ["-Wno-unused-parameter"],

}

```

其余操作和 Android.mk 方式均相同，需要注意模块名的最后两个字符我改成了 bp。



## 参考资料

* [Android系统开发入门-4.添加自定义模块](http://qiushao.net/2019/11/22/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E5%85%A5%E9%97%A8/4-%E6%B7%BB%E5%8A%A0%E8%87%AA%E5%AE%9A%E4%B9%89%E6%A8%A1%E5%9D%97/)
* [Android 10 根文件系统和编译系统(十九)：Android.bp各种模块编译规则](https://blog.csdn.net/ldswfun/article/details/120834205?spm=1001.2014.3001.5502)
* [Soong Modules Reference](https://ci.android.com/builds/submitted/9155974/linux/latest/view/soong_build.html)
