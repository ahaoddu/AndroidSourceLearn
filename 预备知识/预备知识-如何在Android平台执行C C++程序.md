# 预备知识-如何在Android平台执行C/C++程序

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
