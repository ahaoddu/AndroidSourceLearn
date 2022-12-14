## 1. 搭建开发环境

操作系统使用 ubuntu 20.04，根据配置情况使用虚拟机或者裸机安装，建议提高配置使用虚拟机安装。

这里推荐开发机的配置：

- cpu 不低于 6 核心，推荐 8 核心及以上
- 内存不低于 16G，推荐 32G 及以上
- 建议磁盘空间不低于 512 G，推荐使用 ssd 硬盘

这里给个Android源码编译时间参考：
* 我的配置情况是 3800x（8核心16线程） 64G 1T SSD，虚拟机完整编译 Android10 用时在 2 小时左右
* 交流群朋友的配置是 12700 32G 512G ssd，裸机完整编译 Android10 用时在 1 小时左右 

安装好系统后需要安装必要的软件：
```bash
sudo apt-get install git-core gnupg flex bison build-essential zip curl zlib1g-dev gcc-multilib g++-multilib libc6-dev-i386 libncurses5 lib32ncurses5-dev x11proto-core-dev libx11-dev lib32z1-dev libgl1-mesa-dev libxml2-utils xsltproc unzip fontconfig python
```
## 2. 下载源码
Android 的源码分为了两部分，Android 系统源码和 linux 内核源码，需要分开单独下载和编译。

下载 repo 工具
```bash
mkdir ~/bin
curl https://mirrors.tuna.tsinghua.edu.cn/git/git-repo -o ~/bin/repo
chmod +x ~/bin/repo
```

repo 的运行过程中会尝试访问官方的 git 源更新自己，如果想使用 tuna 的镜像源进行更新，可以将如下内容复制到你的 ~/.bashrc 或者 ~/.zshrc 里。

```bash
export REPO_URL='https://mirrors.tuna.tsinghua.edu.cn/git/git-repo'
PATH=~/bin:$PATH
```

初始化仓库并同步远程代码：

```bash
git config --global user.email "you@example.com"
git config --global user.name "Your Name"

mkdir aosp 
cd asop
#初始化仓库,-b 指示分支，这里使用 android10
repo init -u https://mirrors.tuna.tsinghua.edu.cn/git/AOSP/platform/manifest -b android-10.0.0_r41
#同步远程代码
repo sync
```

-b 后面的值参考[源代码标记和 build](https://source.android.com/docs/setup/start/build-numbers?hl=zh-cn#source-code-tags-and-builds)。这里选用了 android-10.0.0_r41 版本用于学习，选择这个版本主要有三个原因：

* Android 版本更新很快，从学习的角度来说，一直追新不切实际，新旧版本原理一致，实现上有差异。学懂了旧版本，工作中使用新版本也是轻车熟路
* 旧版本相较于新版本，参考资料较多
* 我购买了 rk3399 开发板用于学习，该开发板提供的源码为 android-10

如果我们要看其他版本的源码可以这么操作：
```bash
# 查看分支
cd .repo/manifests
git branch -av

# 切换分支
cd aosp #按照你的实际目录执行，进入 aosp 源码目录
rm -rf *
repo init -b android-13.0.0_r3
repo sync
```

## 3. Repo
Repo 是一个 Python 脚本的集合，对 git 操作进行了封装，便于对多个（上百个） git 项目的管理，一般搭配 gerrit code review 工具使用，gerrit 涉及的内容繁多，后面有时间单独来介绍，这里介绍一下 repo 的基本使用。

```bash
#repo管理了多个 git 项目，该命令表示在所有的子项目中开启新的分支，用于添加新的功能,
repo start dev --all
#查看分支
repo branch
#进入我们关系的模块，修改代码，提交
git add .
git commit -m "处理了一个bug"
#上传到审核服务器
repo upload 
```

有的时候，我们修改了代码，又想恢复到初始状态，可以执行下面的命令：
```bash
//在每个项目中运行指定的 shell 命令，慎重使用该命令
repo forall -c git reset --hard 
```
## 4. 编译 Android 源码
```bash
source build/envsetup.sh
lunch aosp_x86_64-eng
# 如果是 Android13
# lunch sdk_phone_x86_64
make -j16
```

## 5. 运行模拟器
```bash
emulator -verbose -cores 4 -show-kernel
```

## 6. 内核

下载适用于模拟器的内核
```bash
git clone https://aosp.tuna.tsinghua.edu.cn/android/kernel/goldfish.git
#查看分支
git branch -a
git checkout android-goldfish-4.14-gchips 
```

编译脚本：

```bash
#!/bin/bash
export ARCH=x86_64
export SUBARCH=x86_64
export CROSS_COMPILE=x86_64-linux-android-
export PATH=android源码目录/prebuilts/gcc/linux-x86/x86/x86_64-linux-android-4.9/bin:$PATH
make x86_64_ranchu_defconfig
make -j16
```

将上面的内容保存为 build.sh 脚本文件。执行 `sh build.sh` 开始编译。

编译有错误修改代码：

* 删除 scripts/selinux/mdp/mdp.c 文件中的  #include <sys/socket.h>
* 删除 scripts/selinux/genheaders/genheaders.c 文件中的  #include <sys/socket.h>
* 在 security/selinux/include/classmap.h 头部添加 #include <sys/socket.h>

执行编译脚本 `sh build.sh` 即可编译成功

## 7. 自定义内核启动
启动之前，需要把之前启动的模拟器和启动模拟器的终端都关掉。

```bash
source build/envsetup.sh
lunch aosp_x86_64-eng
emulator -kernel 内核地址/goldfish/arch/x86/boot/bzImage
```

启动成功，打开模拟器设置页面，进入版本信息。

![image.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6c95abb0c5ba4b668b001b9f3bdcbc77~tplv-k3u1fbpfcp-zoom-1.image)

可以看到 Kernel version 项里，已经是最新编译的内核版本了。

## 参考资料

- [【官方】源代码控制工作流程](https://source.android.com/docs/setup/create/coding-tasks)
- [【官方】编译内核](https://source.android.com/source/building-kernels?hl=zh-cn#id-version)
- [【官方】Repo 命令参考资料 ](https://source.android.com/docs/setup/develop/repo?hl=zh-cn)
- [Android源码模块化管理工具Repo分析](https://juejin.cn/post/6844904148102545416)
- [android ----- goldfish内核编译](https://blog.csdn.net/silently_frog/article/details/124063445)
- [编译错误 error New address family defined, please update secclass_map.解决](https://blog.csdn.net/zhangpengfei991023/article/details/109672491)

## 关于我

- 我叫阿豪，目前定居成都
- 2012 年开始从事 Android 应用和系统开发工作
- 2015 年毕业于国防科技大学指挥自动化工程专业，毕业后一直从事 Android 相关的开发和研究工作
- 如果你对 Android 系统源码感兴趣可以添加我的微信，相互学习交流。

![27c7e38ee991b9d1fb42cb3bdf352a7.jpg](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e740900051964362bd1735e05ed57138~tplv-k3u1fbpfcp-zoom-1.image)
