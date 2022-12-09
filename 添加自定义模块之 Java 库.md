# 添加自定义模块之 Java 库

## 1. Android.mk 方式集成

在 `aosp/device/mycompamy/product/` 目录下创建以下的目录和文件：

```bash
.
├── AndroidProducts.mk
├── lib
│   ├── Android.mk
│   └── com
│       └── ahaoddu
│           └── mytriangle
│               └── Triangle.java
├── main
│   ├── Android.mk
│   └── com
│       └── ahaoddu
│           └── main
│               └── TriangleDemo.java
└── myaosp.mk
```

其中 AndroidProducts.mk myaosp.mk 是 [配置product](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/blob/main/4.Framework%E5%BC%80%E5%8F%91/%E9%85%8D%E7%BD%AEProduct.md) 中添加的 product 配置文件。

### 1.1 java 库

lib 是一个 java 库。其 Android.mk 内容如下：

```makefile
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
## 包含当前目录及子目录下的所有 java 文件
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_MODULE := libmytrianglemk
# 编译到 product 分区,Android 10 下 java 库必须编译到 product 分区才能通过编译
LOCAL_PRODUCT_MODULE := true
# 这是一个 java 库
include $(BUILD_JAVA_LIBRARY)
```

Triangle.java 内容如下：

```cpp
package com.ahaoddu.mytriangle;

public class Triangle
{
	private int a;
	private int b;
	private int c;


	public Triangle(int a, int b, int c)
	{
		this.a = a;
		this.b = b;
		this.c = c;
	}


	public Triangle()
	{
		this(9, 12, 15);
	}

	public int zhouChangFunc()
	{
		return (a+b+c);
	}

	public double areaFunc()
	{
		double p = zhouChangFunc()/2.0;

		return Math.sqrt(p*(p-a)*(p-b)*(p-c));
	}

}
```

接下来我们就可以在 lib 目录下执行 mm 命令，编译当前模块：

```bash
mm
```

### 1.2 main

main 引用了 lib，main目录下：

Android.mk:

```makefile
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := libmytrianglemk 

LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_MODULE := TriangleDemomk
# 编译到 product 分区
LOCAL_PRODUCT_MODULE := true
include $(BUILD_JAVA_LIBRARY)
```

TriangleDemo.java：

```cpp
package com.ahaoddu.main;

import com.ahaoddu.mytriangle.Triangle;

public class  TriangleDemo
{
	public static void main(String[] args) 
	{
		Triangle t1;
		t1 = new Triangle(3, 4, 5);
		System.out.println("t1 area : "+t1.areaFunc());
		System.out.println("t1 round :"+t1.zhouChangFunc());

	}
}

```

同样，在 main 目录下通过 mm 命令编译。

```bash
mm

```

在 `aosp/device/mycompamy/product/myaosp.mk` 中添加：

```bash
PRODUCT_PACKAGES += libmytrianglemk
PRODUCT_PACKAGES += TriangleDemomk
```

接下来编译系统：

```bash
source build/envsetup.sh
lunch myaosp-eng
make -j16
```

编译完成启动虚拟机后：

```bash
# 进入模拟器shell
adb shell
# 配置 classpath
export CLASSPATH=/product/framework/libmytrianglemk.jar:/product/framework/TriangleDemomk.jar
app_process /product/framework/ com.ahaoddu.main.TriangleDemo
```

执行结果如下图所示：
![](https://gitee.com/stingerzou/pic-bed/raw/master/img/20221013123232.png)

## 2. Android.bp 方式集成

将上文的 lib/Android.mk 修改为 Android.bp：

```bash
java_library {
    name: "libmytriangle",
    installable: true,
    product_specific： true,
    srcs: ["**/*.java"],
}
```

product_specific 表示当前模块编译到 product 分区，测试发现，Android10 java lib 必须设置 `product_specific： true` 才能编译通过。

如果不指定 installable: true, 则编译出来的 jar 包里面是 .class 文件。这种包是没法安装到系统上的，只能给其他 java 模块作为 static_libs 依赖。
指定 installable: true, 则编译出来的 jar 包里面是 classes.dex 文件。这种才是 Android 虚拟机可以加载的格式。

main/Android.mk 修改为 Android.bp：

```makefile
java_library {
    name: "libmytrianglebp",
    installable: true,
    product_specific： true,
    srcs: ["**/*.java"],
}
```

将 main/Android.mk 修改为 Android.bp

```makefile
java_library {
    name: "TriangleDemobp",
    installable: true,

    libs: ["libmytriangle"],

    product_specific： true,

    srcs: ["**/*.java"],
}
```

在 myaosp.mk 中添加：

```makefile
PRODUCT_PACKAGES += libmytrianglemk
PRODUCT_PACKAGES += TriangleDemomk
```

其余操作和 Android.mk 方式均相同，需要注意模块名的最后两个字符我改成了 bp。

## 示例源码

示例源码在这个[链接](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/tree/main/4.Framework%E5%BC%80%E5%8F%91/Demos/modules)下的 mk/javalib 和 bp/javalib 目录可以找到。

## 参考资料

* [Android系统开发入门-4.添加自定义模块](http://qiushao.net/2019/11/22/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E5%85%A5%E9%97%A8/4-%E6%B7%BB%E5%8A%A0%E8%87%AA%E5%AE%9A%E4%B9%89%E6%A8%A1%E5%9D%97/)
* [Android 10 根文件系统和编译系统(十九)：Android.bp各种模块编译规则](https://blog.csdn.net/ldswfun/article/details/120834205?spm=1001.2014.3001.5502)
* [Soong Modules Reference](https://ci.android.com/builds/submitted/9155974/linux/latest/view/soong_build.html)
* [Failing builds for Lineage-19.0 due to artifact path requirement](https://github.com/lineageos4microg/android_vendor_partner_gms/issues/5)
* [自定义 SELinux](https://source.android.google.cn/docs/security/selinux/customize?hl=zh-cn)
 