# SEAndroid 入门

## 基本概念

SEAndroid 是一种安全系统，相关的概念和术语对于初学者来说都相对繁琐。我们可以简单地理解：在 Android 系统里面有很多资源，资源主要是根文件系统下的文件，属性系统中的属性，binder 服务，进程，用户等，为了方便描述，我们需要给这些资源取名字，资源的名字在 SEAndroid 中称之为安全上下文（security context）：

```bash
# 这里的意思是 /dev/myse_dev 的名字（security context）是 u:object_r:myse_testdev_t:s0
# myse_testdev_t 前后的内容为固定格式，暂时不关心其作用
/dev/myse_dev u:object_r:myse_testdev_t:s0
```

security context 由分号分为了4个部分，第 1，4 部分内容一般固定，暂时不管它们。对于进程（活的）第 2 部分会定义为 r，对于文件（死的）会定义为 object_r。

第 3 部分，对于文件来说是 type，对于进程是 domain，内容为用户自定义，表示该安全上下文的类型。type 就是用来定义安全上下文类型的：

```bash
# 定义一个安全上下文类型 myse_testdev_t，并定义其类型为 dev_type，表示这是一个设备类型
type myse_testdev_t, dev_type;
```

这里的 dev_type 可以认为是安全上下文类型的类型，通过 attribute 定义：

```bash
# 定义一个类型 dev_type，表示设备类型  来自 system/sepolicy/public/attributes
attribute dev_type
```

当定义好安全上下文后，我们需要将安全上下文（资源的名字）与具体的资源相关联：

```bash
# 这里的意思是 /dev/myse_dev 的名字（security context）是 u:object_r:myse_testdev_t:s0
/dev/myse_dev u:object_r:myse_testdev_t:s0
```

以上的准备工作完成后，我们就可以添加相应的规则来限制资源的访问：

```bash
# 示例来自于 system/sepolicy/private/adbd.te 
# 允许 adbd （安全上下文）， 对安全上下文为 anr_data_file 的目录(dir)有读目录权限
allow adbd anr_data_file:dir r_dir_perms;
```

其中 adbd 称之为主体，anr_data_file 称为客体，dir 表示客体的类型是目录，其定义位于：`system/sepolicy/private/security_classes` :

```bash
class security
class process
class system
class capability

# file-related classes
class filesystem
class file
class dir
class fd
class lnk_file
class chr_file
class blk_file
class sock_file
class fifo_file

# network-related classes
class socket
class tcp_socket
class udp_socket
class rawip_socket
class node
class netif
class netlink_socket
class packet_socket
class key_socket
class unix_stream_socket
class unix_dgram_socket

# 省略
#.......
```

r_dir_perms 表示目录的读权限，定义在 `system/sepolicy/public/global_macros`：

```bash
# ......
define(`x_file_perms', `{ getattr execute execute_no_trans map }')
define(`r_file_perms', `{ getattr open read ioctl lock map }')
define(`w_file_perms', `{ open append write lock map }')
define(`rx_file_perms', `{ r_file_perms x_file_perms }')
define(`ra_file_perms', `{ r_file_perms append }')
define(`rw_file_perms', `{ r_file_perms w_file_perms }')
define(`rwx_file_perms', `{ rw_file_perms x_file_perms }')
define(`create_file_perms', `{ create rename setattr unlink rw_file_perms }')
#......
```

## 示例

SEAndroid 相关的内容繁多，一般都是参考源码拷贝粘贴，要完全掌握其细节费神费力，一般通过示例来学习和积累。这里先演示一个访问设备文件的 CPP 可执行程序。

在 `aosp/device/mycompamy/product` 目录下添加如下的文件和文件夹：

```bash
.
├── AndroidProducts.mk
├── myaosp.mk
└── test_se
    ├── cmd
    │   ├── Android.mk
    │   └── myse_test.c
    └── sepolicy
        ├── device.te
        ├── file_contexts
        └── myse_test.te
```

cmd 目录下是一个读取文件的可执行程序：

myse_test.c:
```c

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define LOG_TAG "SeTest"
#include <log/log.h>

int main(int argc, char *argv[])
{
	
	int fd  = -1;
	int ret = -1;
	char *content = "hello test for selinux";
	char *dev_name = "/dev/myse_dev";
	fd = open(dev_name, O_RDWR);
	if(fd < 0)
	{
		ALOGE("open %s error: %s", dev_name, strerror(errno));
		return -1;
	}

	ret = write(fd, content, strlen(content));	
	if(ret < 0)
	{
		ALOGE("write testfile error: %s", strerror(errno));
		return -1;
	}else
	{
		ALOGD("write testfile ok: %d",  ret);
	}
	
	while(1);

	close(fd);

	return 0;
}

```

这段程序的主要作用就是向 `/dev/myse_dev` 文件写入一段字符串。

Android.mk：

```makefile
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    myse_test.c

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \

LOCAL_CFLAGS += -Wno-unused-parameter
#写到 vendor 分区
LOCAL_VENDOR_MODULE := true
# 貌似可执行程序写到 product 分区，selinux 权限会有一些问题
# 知道的朋友请告知一下
#LOCAL_PRODUCT_MODULE := true
LOCAL_MODULE:= myse_test

include $(BUILD_EXECUTABLE)
```

myse_test.te：

```bash
# subject context in proccess status
type  myse_test_dt, domain;
# object context as a file
type myse_test_dt_exec, exec_type, vendor_file_type, file_type;
#表示该程序如果从 init 进程启动，其安全上下文的 domain 部分从 init 转化为 myse_test_dt
init_daemon_domain(myse_test_dt);
#从 shell 启动 type 为 myse_test_dt_exec 的可执行程序，其对应进程的 domain 为 myse_test_dt
domain_auto_trans(shell, myse_test_dt_exec, myse_test_dt);
```

device.te：

```bash
# 定义设备 /dev/myse_dev 的类型
type myse_testdev_t, dev_type;
```

file_contexts：

```bash
/vendor/bin/myse_test                   u:object_r:myse_test_dt_exec:s0

/dev/myse_dev    u:object_r:myse_testdev_t:s0

```

编译 user-debug 版本,使得运行环境与最终 user 版接近并且可以在 shell 环境方便切换到 root 用户：

```bash
# aosp 目录下
source build/envsetup.sh
lunch myaosp-userdebug
make -j16
```

准备工作：

```bash
#进入Android shell 环境
adb shell
#切换到 root 用户
su 
# 创建待访问的设备文件
touch /dev/myse_dev
ls -Z /dev/myse_dev                     
u:object_r:device:s0 /dev/myse_dev
# 加载 file_contexts
restorecon /dev/myse_dev
ls -Z /dev/myse_dev            
u:object_r:myse_testdev_t:s0 /dev/myse_dev
#放宽权限
chmod 777 /dev/myse_dev 
# 可执行文件
ls -Z /vendor/bin/myse_test
u:object_r:myse_test_dt_exec:s0 /vendor/bin/myse_test
```

开始执行程序：

```bash
# 切换到selinux permissive 模式，该模式下不会阻止进程的行为，只会打印权限缺失信息
setenforce 0
# 执行程序
myse_test
```

再开一个终端看log：

```bash
logcat | grep myse_test               
10-18 15:52:32.980  3134  3134 W myse_test: type=1400 audit(0.0:17): avc: denied { use } for path="/dev/pts/0" dev="devpts" ino=3 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=fd permissive=0
10-18 15:52:32.980  3134  3134 W myse_test: type=1400 audit(0.0:19): avc: denied { use } for path="/dev/pts/0" dev="devpts" ino=3 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=fd permissive=0
10-18 15:52:32.980  3134  3134 W myse_test: type=1400 audit(0.0:20): avc: denied { use } for path="/dev/goldfish_pipe" dev="tmpfs" ino=6954 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=fd permissive=0
10-18 15:52:32.980  3134  3134 W myse_test: type=1400 audit(0.0:21): avc: denied { use } for path="socket:[9874]" dev="sockfs" ino=9874 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=fd permissive=0
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:30): avc: denied { use } for path="/dev/pts/0" dev="devpts" ino=3 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=fd permissive=1
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:31): avc: denied { read write } for path="/dev/pts/0" dev="devpts" ino=3 scontext=u:r:myse_test_dt:s0 tcontext=u:object_r:devpts:s0 tclass=chr_file permissive=1
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:32): avc: denied { read write } for path="socket:[9874]" dev="sockfs" ino=9874 scontext=u:r:myse_test_dt:s0 tcontext=u:r:adbd:s0 tclass=unix_stream_socket permissive=1
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:33): avc: denied { use } for path="/vendor/bin/myse_test" dev="dm-1" ino=140 scontext=u:r:myse_test_dt:s0 tcontext=u:r:shell:s0 tclass=fd permissive=1
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:34): avc: denied { read write } for name="myse_dev" dev="tmpfs" ino=27818 scontext=u:r:myse_test_dt:s0 tcontext=u:object_r:device:s0 tclass=file permissive=1
10-18 15:54:27.340  3710  3710 I myse_test: type=1400 audit(0.0:35): avc: denied { open } for path="/dev/myse_dev" dev="tmpfs" ino=27818 scontext=u:r:myse_test_dt:s0 tcontext=u:object_r:device:s0 tclass=file permissive=1
```

我们把权限相关的log复制下来，在 aosp 目录下，保存到 avc_log.txt 文件中，并执行一下命令：

```bash
audit2allow -i avc_log.txt


#============= myse_test_dt ==============
allow myse_test_dt adbd:fd use;
allow myse_test_dt adbd:unix_stream_socket { read write };
allow myse_test_dt device:file { open read write };
allow myse_test_dt devpts:chr_file { read write };
allow myse_test_dt shell:fd use;
```

这里就会输出相应的权限规则,我们将其添加到源码中 myse_test.te 后面即可：

```bash
# subject context in proccess status
type  myse_test_dt, domain;
# object context as a file
type myse_test_dt_exec, exec_type, vendor_file_type, file_type;
#grant perm as domain
init_daemon_domain(myse_test_dt);

domain_auto_trans(shell, myse_test_dt_exec, myse_test_dt);

============= myse_test_dt ==============
allow myse_test_dt adbd:unix_stream_socket { read write };
allow myse_test_dt device:file { open read write };
allow myse_test_dt devpts:chr_file { read write };
allow myse_test_dt shell:fd use;
```

再次编译运行系统，即可正常使用 myse_test 程序


## 源码

 https://github.com/ahaoddu/AndroidKnowledgeHierarchy/tree/main/4.Framework%E5%BC%80%E5%8F%91/Demos/test_se


## 参考资料
* [SEAndroid安全机制简要介绍和学习计划](https://blog.csdn.net/Luoshengyang/article/details/35392905?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522166579375216800180692092%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=166579375216800180692092&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-3-35392905-null-null.nonecase&utm_term=selinux&spm=1018.2226.3001.4450)
* [Android系统10 RK3399 init进程启动(二十一) DAC和MAC介绍](https://blog.csdn.net/ldswfun/article/details/124637485?spm=1001.2014.3001.5502)
* [构建 SELinux 政策](https://source.android.com/docs/security/features/selinux/build?hl=zh-cn)
* [自定义 SELinux](https://source.android.google.cn/docs/security/selinux/customize?hl=zh-cn)