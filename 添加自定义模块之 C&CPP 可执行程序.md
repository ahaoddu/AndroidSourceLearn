# 添加自定义模块之 C&CPP 可执行程序

自定义模块就是我们添加到 Android 系统中的：

* C/C++ 可执行程序
* C/C++ 库
* Java库
* Android 库
* apk
* ......

刚开始，Android 系统使用 Android.mk 的方式来添加模块，Android.mk 实质是 Makefile 脚本，随着 Android 系统日趋复杂，编译速度越来越慢（我的 ryzen 3800x 编译 Android5 只要30分钟，Android10 已经要 2 小时了），google 添加了 Android.bp 的方式来添加模块，据说编译会更快（换 cpu 才是加快编译速度的最佳方案，^_^）。随着系统的更新，Android.mk 会越来越少，Android.bp 会成为未来的主流，当然 Android.mk 也要能看懂。本文会介绍两种方式来添加模块。

这里我们先来看看 C&CPP 可执行程序

## 1. Android.mk 方式集成

在 `aosp/device/mycompamy/product/` 目录下创建 hello 目录，在 hello 目录内添加 hello.cpp :

```c++
#include <cstdio>

int main()
{
    printf("Hello Android\n");
    return 0;
}

```

在 hello 目录内添加 android.mk：

```bash
# 固定内容
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# c flag
LOCAL_CFLAGS += \
                -Wno-error \
                -Wno-unused-parameter


# user: 指该模块只在user版本下才编译
# eng: 指该模块只在eng版本下才编译
# tests: 指该模块只在tests版本下才编译
# optional:指该模块在所有版本下都编译
LOCAL_MODULE_TAGS := optional
# "both": build both 32-bit and 64-bit.
# "32": build only 32-bit.
# "64": build only 64-bit.
LOCAL_MULTILIB := 64

# 编译到 vender 而不是 system
LOCAL_VENDOR_MODULE := true

# 源码
LOCAL_SRC_FILES := hello.cpp

# 模块名
LOCAL_MODULE := hellomk
# 表示当前模块是可执行程序
include $(BUILD_EXECUTABLE)
```

在 `aosp/device/mycompamy/product/myaosp.mk` 中添加：

```bash
PRODUCT_PACKAGES += hellomk
```

接下来编译系统：

```bash
source build/envsetup.sh
lunch myaosp-eng
make -j16
```

编译完成启动虚拟机后，就可以通过 adb shell 运行我们的 hello 程序了

```bash
adb shell hellomk
```

执行结果如下图所示：

![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221012101831.png)

这里解释一下 LOCAL_VENDOR_MODULE：

自从 Android 8 推出 treble 计划， 谷歌在升级系统的时候， 希望硬件厂商能够同步和快速的适配新的系统， 谷歌将自己的代码和厂商的代码进行隔离， 谷歌自己开发的核心代码放在 system 分区， 厂商的放在 vendor 分区，两个分区的可执行程序和动态库的是隔离， vendor 分区的可执行程序不能随意调用 system 分区中的动态库，所以在编译的时候， 我们的源码会选择性的放在 system 分区还是 vendor 分区。LOCAL_VENDOR_MODULE 变量为 true 当前模块就编译到 vender 目录下，否则就编译到 system 目录下。

## 2. Android.bp 方式集成

将上文的 Android.mk 修改为 Android.bp：

```bash
cc_binary {              //模块类型为可执行文件
    name: "hellobp",       //模块名hellobp
    srcs: ["hello.cpp"], //源文件列表
    vendor: true,        //编译出来放在/vendor目录下(默认是放在/system目录下)
    cflags: ["-Werror"], //添加编译选项
}
```

这里的 vender 和 Android.mk 中 LOCAL_VENDOR_MODULE 作用相同。

修改 `aosp/device/mycompamy/product/myaosp.mk`

```bash
PRODUCT_PACKAGES += hellobp
```

其余操作和 Android.mk 方式均相同。

## 示例源码

示例源码在这个[链接](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/tree/main/4.Framework%E5%BC%80%E5%8F%91/Demos/modules)下的 mk/executable 和 bp/excutable 目录可以找到。

https://blog.csdn.net/liaosongmao1/article/details/124843774

https://github.com/lineageos4microg/android_vendor_partner_gms/issues/5

## 参考资料

* [Android系统开发入门-4.添加自定义模块](http://qiushao.net/2019/11/22/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E5%85%A5%E9%97%A8/4-%E6%B7%BB%E5%8A%A0%E8%87%AA%E5%AE%9A%E4%B9%89%E6%A8%A1%E5%9D%97/)
* [Android 10 根文件系统和编译系统(十九)：Android.bp各种模块编译规则](https://blog.csdn.net/ldswfun/article/details/120834205?spm=1001.2014.3001.5502)
* [Soong Modules Reference](https://ci.android.com/builds/submitted/9155974/linux/latest/view/soong_build.html)
