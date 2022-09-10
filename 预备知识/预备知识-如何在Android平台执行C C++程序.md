# 预备知识-如何在Android平台执行C/C++程序

这是一个系列教程，陆续更新中：<br />

[预备知识-如何在Android平台执行C/C++程序](https://github.com/dducd/AndroidSourceLearn/blob/main/%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86/%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86-%E5%A6%82%E4%BD%95%E5%9C%A8Android%E5%B9%B3%E5%8F%B0%E6%89%A7%E8%A1%8CC%20C%2B%2B%E7%A8%8B%E5%BA%8F.md)

[1.Android源码分析与实践-环境准备与源码下载](https://github.com/dducd/AndroidSourceLearn/blob/main/1.Android%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90%E4%B8%8E%E5%AE%9E%E8%B7%B5-%E7%8E%AF%E5%A2%83%E5%87%86%E5%A4%87%E4%B8%8E%E6%BA%90%E7%A0%81%E4%B8%8B%E8%BD%BD.md)

[2.Linux驱动入门-模块](https://github.com/dducd/AndroidSourceLearn/blob/main/2.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E6%A8%A1%E5%9D%97.md)

下图是计划中的路线图，陆续更新中，后续根据实际情况会做一些微调。<br />![image.png](https://cdn.nlark.com/yuque/0/2022/png/2613680/1662521721762-5945250e-512b-4dd6-a88f-62673e255774.png#clientId=u78706d6f-6b62-4&crop=0&crop=0&crop=1&crop=1&from=paste&height=1180&id=u7a57284c&name=image.png&originHeight=1770&originWidth=1260&originalType=binary&ratio=1&rotation=0&showTitle=false&size=106865&status=done&style=none&taskId=u5e8f5fca-1118-4ba8-8a08-9bbad290e9a&title=&width=840)
<a name="6483e310"></a>

我们直接看一个示例：

写一个 helloworld c++ 可执行程序
```cpp
# include <iostream>

int main(int argc, char const *argv[])
{
  for(int i = 0; i < 5; ++i)
    std::cout << "Hello World" << std::endl;

  return 0;
}

```

写 CMakeLists.txt：
google 给了两种方式用于支持 CMake 调用 NDK 的工具链编译 C/C++ 代码，
一种是 [CMake的内置支持](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk)，需要 CMake 版本 >= 3.21，NDK 版本需要大于 r23，是未来的主流。
一种是通过[工具链文件支持](https://developer.android.com/ndk/guides/cmake)，是当前的主流。Android Gradle 插件使用的是 NDK 的工具链文件。
本文采用第二种方式：

CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.0)

project(test)

add_executable(${PROJECT_NAME} hello_drv_test.c)
```

编译脚本 build.sh:
```cmake
export ANDROID_NDK=你的ndk完整路径

rm -r build
mkdir build && cd build 

# CMake的内置支持
# cmake -DCMAKE_SYSTEM_NAME=Android \
# 	-DCMAKE_SYSTEM_VERSION=29 \
# 	-DCMAKE_ANDROID_ARCH_ABI=x86_64 \
# 	-DANDROID_NDK=$ANDROID_NDK \
# 	-DCMAKE_ANDROID_STL_TYPE=c++_shared \
# 	..

# 工具链文件支持
cmake \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=x86_64 \
    -DANDROID_PLATFORM=android-29 \
	-DANDROID_STL=c++_shared \
	..

cmake --build .
```

在模拟器上执行我们的程序：

```cmake
# 编译程序
chmod +x build.sh
./build.sh
# 打开模拟器，流程略
# 上传可执行文件
adb push build/test /data/local/tmp
# 上传 STL 动态库
adb push 你的ndk完整路径/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/x86_64-linux-android/libc++_shared.so /data/local/tmp
# 进入到模拟器 shell
adb shell
# 执行程序
cd /data/local/tmp
export LD_LIBRARY_PATH=/data/local/tmp && ./test
```
## 参考资料

- [[CMake 文档] Cross Compiling for Android with the NDK](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android-with-the-ndk)
- [Google CMake 文档](https://developer.android.com/ndk/guides/cmake)
