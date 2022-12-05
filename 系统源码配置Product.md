# 系统源码配置 Product

## 1. 什么是 Product

我们再编译源码之前需要执行 `lunch` ：

```bash
➜  aosp lunch

You're building on Linux

Lunch menu... pick a combo:
     1. aosp_arm-eng
     2. aosp_arm64-eng
     3. aosp_blueline-userdebug
     4. aosp_bonito-userdebug
     5. aosp_car_arm-userdebug
     6. aosp_car_arm64-userdebug
     7. aosp_car_x86-userdebug
     8. aosp_car_x86_64-userdebug
     9. aosp_cf_arm64_phone-userdebug
     10. aosp_cf_x86_64_phone-userdebug
     11. aosp_cf_x86_auto-userdebug
     12. aosp_cf_x86_phone-userdebug
     13. aosp_cf_x86_tv-userdebug
     14. aosp_coral-userdebug
     15. aosp_coral_car-userdebug
     16. aosp_crosshatch-userdebug
     17. aosp_crosshatch_car-userdebug
     18. aosp_flame-userdebug
     19. aosp_marlin-userdebug
     20. aosp_sailfish-userdebug
     21. aosp_sargo-userdebug
     22. aosp_taimen-userdebug
     23. aosp_walleye-userdebug
     24. aosp_walleye_test-userdebug
     25. aosp_x86-eng
     26. aosp_x86_64-eng
     27. beagle_x15-userdebug
     28. car_x86_64-userdebug
     29. fuchsia_arm64-eng
     30. fuchsia_x86_64-eng
     31. hikey-userdebug
     32. hikey64_only-userdebug
     33. hikey960-userdebug
     34. hikey960_tv-userdebug
     35. hikey_tv-userdebug
     36. m_e_arm-userdebug
     37. mini_emulator_arm64-userdebug
     38. mini_emulator_x86-userdebug
     39. mini_emulator_x86_64-userdebug
     40. poplar-eng
     41. poplar-user
     42. poplar-userdebug
     43. qemu_trusty_arm64-userdebug
     44. uml-userdebug

Which would you like? [aosp_arm-eng]
```

这里的每一个选项就是一个 Product，一份源码可以配置多个 Product 。我们对 framework 做定制的第一步就是添加一个自己的 Product。

## 2. Product 配置文件

### 2.1 配置文件的结构

Product 配置文件保存在以下两个目录：

* build/target：aosp 提供的 product 配置文件保存在这个目录下
* device：芯片厂商提供的 product 配置文件保存在这个目录下

这里看一下 build/target 目录下的结构：

```bash
➜  aosp tree build/target -L 2
build/target
├── board
│   ├── Android.mk
│   ├── BoardConfigEmuCommon.mk
│   ├── BoardConfigGsiCommon.mk
│   ├── BoardConfigMainlineCommon.mk
│   ├── generic
│   ├── generic_arm64
│   ├── generic_arm64_ab
│   ├── generic_arm_ab
│   ├── generic_x86
│   ├── generic_x86_64
│   ├── generic_x86_64_ab
│   ├── generic_x86_ab
│   ├── generic_x86_arm
│   ├── go_defaults_512.prop
│   ├── go_defaults_common.prop
│   ├── go_defaults.prop
│   ├── gsi_arm64
│   ├── gsi_system.prop
│   ├── gsi_system_user.prop
│   └── mainline_arm64
├── OWNERS
└── product
    ├── AndroidProducts.mk
    ├── aosp_arm64_ab.mk
    ├── aosp_arm64.mk
    ├── aosp_arm_ab.mk
    ├── aosp_arm.mk
    ├── aosp_base.mk
    ├── aosp_base_telephony.mk
    ├── aosp_x86_64_ab.mk
    ├── aosp_x86_64.mk
    ├── aosp_x86_ab.mk
    ├── aosp_x86_arm.mk
    ├── aosp_x86.mk
    ├── base.mk
    ├── base_product.mk
    ├── base_system.mk
    ├── base_vendor.mk
    ├── cfi-common.mk
    ├── core_64_bit.mk
    ├── core_64_bit_only.mk
    ├── core_minimal.mk
    ├── developer_gsi_keys.mk
    ├── empty-preloaded-classes
    ├── empty-profile
    ├── emulator.mk
    ├── emulator_system.mk
    ├── emulator_vendor.mk
    ├── full_base.mk
    ├── full_base_telephony.mk
    ├── full.mk
    ├── full_x86.mk
    ├── generic.mk
    ├── generic_no_telephony.mk
    ├── generic_x86.mk
    ├── go_defaults_512.mk
    ├── go_defaults_common.mk
    ├── go_defaults.mk
    ├── gsi
    ├── gsi_arm64.mk
    ├── gsi_common.mk
    ├── gsi_keys.mk
    ├── handheld_product.mk
    ├── handheld_system.mk
    ├── handheld_vendor.mk
    ├── languages_default.mk
    ├── languages_full.mk
    ├── legacy_gsi_common.mk
    ├── mainline_arm64.mk
    ├── mainline.mk
    ├── mainline_system_arm64.mk
    ├── mainline_system.mk
    ├── media_product.mk
    ├── media_system.mk
    ├── media_vendor.mk
    ├── OWNERS
    ├── product_launched_with_k.mk
    ├── product_launched_with_l.mk
    ├── product_launched_with_l_mr1.mk
    ├── product_launched_with_m.mk
    ├── product_launched_with_n.mk
    ├── product_launched_with_n_mr1.mk
    ├── product_launched_with_o.mk
    ├── product_launched_with_o_mr1.mk
    ├── product_launched_with_p.mk
    ├── profile_boot_common.mk
    ├── runtime_libart.mk
    ├── sdk_arm64.mk
    ├── sdk.mk
    ├── sdk_phone_arm64.mk
    ├── sdk_phone_armv7.mk
    ├── sdk_phone_x86_64.mk
    ├── sdk_phone_x86.mk
    ├── sdk_x86_64.mk
    ├── sdk_x86.mk
    ├── security
    ├── telephony.mk
    ├── telephony_product.mk
    ├── telephony_system.mk
    ├── telephony_vendor.mk
    ├── updatable_apex.mk
    ├── vboot.mk
    └── verity.mk
```

board目录下主要是一些开发版相关的配置， product 目录主要是产品相关的配置，我们主要关注一下几个文件：

* `/board/generic_x86_64/BoardConfig.mk` ： 用于配置开发板
* `/product/AndroidProducts.mk`   `/product/aosp_x86_64.mk`：用于配置 Product

我们自定义的 Product 一般放在 device 目录下，其结构一般如下：

```bash
device
└── company_name
    ├── product_device_name
    │   └── BoardConfig.mk
    └── product
        ├── AndroidProducts.mk
        ├── first_product_name.mk
        └── second_product_name.mk
```

### 2.2 配置文件详解

#### **2.2.1 BoardConfig.mk**

用于定义和硬件相关的底层特性和变量，比如当前源码支持的 cpu 位数(64/32位)，bootloader 和 kernel, 是否支持摄像头，GPS导航等一些板级特性。

build/target/board/generic_x86_64/BoardConfig.mk 的具体内容如下:

```bash
# x86_64 emulator specific definitions
TARGET_CPU_ABI := x86_64
TARGET_ARCH := x86_64
TARGET_ARCH_VARIANT := x86_64

TARGET_2ND_CPU_ABI := x86
TARGET_2ND_ARCH := x86
TARGET_2ND_ARCH_VARIANT := x86_64

TARGET_PRELINK_MODULE := false
include build/make/target/board/BoardConfigGsiCommon.mk
include build/make/target/board/BoardConfigEmuCommon.mk

BOARD_USERDATAIMAGE_PARTITION_SIZE := 576716800

BOARD_SEPOLICY_DIRS += device/generic/goldfish/sepolicy/x86

# Wifi.
BOARD_WLAN_DEVICE           := emulator
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_simulated
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_simulated
WPA_SUPPLICANT_VERSION      := VER_0_8_X
WIFI_DRIVER_FW_PATH_PARAM   := "/dev/null"
WIFI_DRIVER_FW_PATH_STA     := "/dev/null"
WIFI_DRIVER_FW_PATH_AP      := "/dev/null"
```

#### 2.2.2 AndroidProducts.mk

AndroidProducts.mk 文件用于描述产品列表， 表示当前公司有哪些产品， 主要包括 PRODUCT_MAKEFILES 变量， 会记录各个产品所用的 Makefile 的列表。同时在新的 Android 系统版本中可以添加 COMMON_LUNCH_CHOICES 编译， 用于在 lunch 命令之后显示的产品选项。

build/target/product/AndroidProducts.mk 的具体内容如下：

```bash
# Unbundled apps will be built with the most generic product config.
ifneq ($(TARGET_BUILD_APPS),)
PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/aosp_arm64.mk \
    $(LOCAL_DIR)/aosp_arm.mk \
    $(LOCAL_DIR)/aosp_x86_64.mk \
    $(LOCAL_DIR)/aosp_x86.mk \
    $(LOCAL_DIR)/full.mk \
    $(LOCAL_DIR)/full_x86.mk \

else
PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/aosp_arm64_ab.mk \
    $(LOCAL_DIR)/aosp_arm64.mk \
    $(LOCAL_DIR)/aosp_arm_ab.mk \
    $(LOCAL_DIR)/aosp_arm.mk \
    $(LOCAL_DIR)/aosp_x86_64_ab.mk \
    $(LOCAL_DIR)/aosp_x86_64.mk \
    $(LOCAL_DIR)/aosp_x86_ab.mk \
    $(LOCAL_DIR)/aosp_x86_arm.mk \
    $(LOCAL_DIR)/aosp_x86.mk \
    $(LOCAL_DIR)/full.mk \
    $(LOCAL_DIR)/full_x86.mk \
    $(LOCAL_DIR)/generic.mk \
    $(LOCAL_DIR)/generic_x86.mk \
    $(LOCAL_DIR)/gsi_arm64.mk \
    $(LOCAL_DIR)/mainline_arm64.mk \
    $(LOCAL_DIR)/mainline_system_arm64.mk \
    $(LOCAL_DIR)/sdk_arm64.mk \
    $(LOCAL_DIR)/sdk.mk \
    $(LOCAL_DIR)/sdk_phone_arm64.mk \
    $(LOCAL_DIR)/sdk_phone_armv7.mk \
    $(LOCAL_DIR)/sdk_phone_x86_64.mk \
    $(LOCAL_DIR)/sdk_phone_x86.mk \
    $(LOCAL_DIR)/sdk_x86_64.mk \
    $(LOCAL_DIR)/sdk_x86.mk \

endif

COMMON_LUNCH_CHOICES := \
    aosp_arm64-eng \
    aosp_arm-eng \
    aosp_x86_64-eng \
    aosp_x86-eng \
```

#### 2.2.3 product_name.mk

某个产品的 Makefile，用于配置默认自带的软件，厂商名，产品名称，设备名称等信息。 同时也可以定义PRODUCT_COPY_FILES， PRODUCT_PROPERTY_OVERRIDES，PRODUCT_PACKAGES，对设备中需要用到的特有文件进行预编译和定制化。

build/target/product/aosp_x86_64.mk 的内容如下：

```bash
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# The system image of aosp_x86_64-userdebug is a GSI for the devices with:
# - x86 64 bits user space
# - 64 bits binder interface
# - system-as-root
# - VNDK enforcement
# - compatible property override enabled

# This is a build configuration for a full-featured build of the
# Open-Source part of the tree. It's geared toward a US-centric
# build quite specifically for the emulator, and might not be
# entirely appropriate to inherit from for on-device configurations.

# GSI for system/product
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/gsi_common.mk)

# Emulator for vendor
$(call inherit-product-if-exists, device/generic/goldfish/x86_64-vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulator_vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/board/generic_x86_64/device.mk)

# Enable mainline checking for excat this product name
ifeq (aosp_x86_64,$(TARGET_PRODUCT))
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS := relaxed
endif

PRODUCT_ARTIFACT_PATH_REQUIREMENT_WHITELIST += \
    root/init.zygote32_64.rc \
    root/init.zygote64_32.rc \

# Copy different zygote settings for vendor.img to select by setting property
# ro.zygote=zygote64_32 or ro.zygote=zygote32_64:
#   1. 64-bit primary, 32-bit secondary OR
#   2. 32-bit primary, 64-bit secondary
# init.zygote64_32.rc is in the core_64_bit.mk below
PRODUCT_COPY_FILES += \
    system/core/rootdir/init.zygote32_64.rc:root/init.zygote32_64.rc

PRODUCT_NAME := aosp_x86_64
PRODUCT_DEVICE := generic_x86_64
PRODUCT_BRAND := Android
PRODUCT_MODEL := AOSP on x86_64
```

### 2.3 配置文件中的函数与变量

#### 2.3.1 inherit-product 函数

inherit-product 函数表示继承另外一个文件：

```bash
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulator_vendor.mk)
$(call inherit-product-if-exists, device/generic/goldfish/x86_64-vendor.mk)
```

#### 2.3.2  include

在Makefile中可使用“-include”来代替“include”，来忽略由于包含文件不存在或者无法创建时的错误提示（“-”的意思是告诉make，忽略此操作的错误。make继续执行）,不加-,当include的文件出错或者不存在的时候， make 会报错并退出。

```bash
-include $(TARGET_DEVICE_DIR)/AndroidBoard.mk
```

include 和 inherit-product 的区别：

假设您 PRODUCT_VAR := a在 A.mk 中，PRODUCT_VAR := b 在 B.mk 中。

如果你在 A.mk 中include  B.mk，你最终会得到 PRODUCT_VAR := b。

但是如果你 inherit-product 在 A.mk 中 B.mk，你会得到 PRODUCT_VAR := a b.

并 inherit-product 确保您不会两次包含 makefile 。

#### 2.3.3 变量

通用变量：

```bash
PRODUCT_BRAND := Android

PRODUCT_NAME := sdk_phone_x86_64

PRODUCT_DEVICE := generic_x86_64

PRODUCT_MODEL := Android SDK built for x86_64
```

几个常用的路径变量：

* SRC_TARGET_DIR 其值为 build/target
* LOCAL_DIR 代表当前目录

自定义变量：表示该变量如何使用， 取决于自己，如

```bash
BOARD_DDR_VAR_ENABLED := true
```

功能变量：表示改变量有特殊功能

* PRODUCT_COPY_FILES： 用于完成拷贝

  ```bash
  PRODUCT_COPY_FILES += vendor/rockchip/common/phone/etc/spn-conf.xml:system/etc/spn-conf.xml
  ```
* PRODUCT_PROPERTY_OVERRIDES： 用于设置系统属性(覆盖)

  ```bash
  PRODUCT_PROPERTY_OVERRIDES += \
      ro.product.version = 1.0.0 \
  ```

## 3. 添加自己的 Product

在 device 目录下添加如下的目录与文件：

```bash
mycompany
├── board
│   └── my_generic_x86_64
│       └── BoardConfig.mk
└── product
    ├── AndroidProducts.mk
    └── myaosp.mk
```

将 build/target/board/generic_x86_64/BoardConfig.mk 的内容拷贝到 BoardConfig.mk ：

```bash
# x86_64 emulator specific definitions
TARGET_CPU_ABI := x86_64
TARGET_ARCH := x86_64
TARGET_ARCH_VARIANT := x86_64

TARGET_2ND_CPU_ABI := x86
TARGET_2ND_ARCH := x86
TARGET_2ND_ARCH_VARIANT := x86_64

TARGET_PRELINK_MODULE := false
include build/make/target/board/BoardConfigGsiCommon.mk
include build/make/target/board/BoardConfigEmuCommon.mk

BOARD_USERDATAIMAGE_PARTITION_SIZE := 576716800

BOARD_SEPOLICY_DIRS += device/generic/goldfish/sepolicy/x86

# Wifi.
BOARD_WLAN_DEVICE           := emulator
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_simulated
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_simulated
WPA_SUPPLICANT_VERSION      := VER_0_8_X
WIFI_DRIVER_FW_PATH_PARAM   := "/dev/null"
WIFI_DRIVER_FW_PATH_STA     := "/dev/null"
WIFI_DRIVER_FW_PATH_AP      := "/dev/null"
```

将 build/target/product/aosp_x86_64.mk 拷贝到 myaosp.mk ，并修改最后四行：

```bash
#
# Copyright 2013 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PRODUCT_USE_DYNAMIC_PARTITIONS := true

# The system image of aosp_x86_64-userdebug is a GSI for the devices with:
# - x86 64 bits user space
# - 64 bits binder interface
# - system-as-root
# - VNDK enforcement
# - compatible property override enabled

# This is a build configuration for a full-featured build of the
# Open-Source part of the tree. It's geared toward a US-centric
# build quite specifically for the emulator, and might not be
# entirely appropriate to inherit from for on-device configurations.

# GSI for system/product
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/gsi_common.mk)

# Emulator for vendor
$(call inherit-product-if-exists, device/generic/goldfish/x86_64-vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulator_vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/board/generic_x86_64/device.mk)

# Enable mainline checking for excat this product name
#ifeq (aosp_x86_64,$(TARGET_PRODUCT))
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS := relaxed
#endif

PRODUCT_ARTIFACT_PATH_REQUIREMENT_WHITELIST += \
    root/init.zygote32_64.rc \
    root/init.zygote64_32.rc \

# Copy different zygote settings for vendor.img to select by setting property
# ro.zygote=zygote64_32 or ro.zygote=zygote32_64:
#   1. 64-bit primary, 32-bit secondary OR
#   2. 32-bit primary, 64-bit secondary
# init.zygote64_32.rc is in the core_64_bit.mk below
PRODUCT_COPY_FILES += \
    system/core/rootdir/init.zygote32_64.rc:root/init.zygote32_64.rc
PRODUCT_NAME := myaosp
PRODUCT_DEVICE := my_generic_x86_64
PRODUCT_BRAND := Android
PRODUCT_MODEL := AOSP on x86_64

```

其中 `PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS := relaxed` 上下的 if 语句需要注释掉。

在 AndroidProducts.mk 中添加内容：

```bash
PRODUCT_MAKEFILES :=\
	$(LOCAL_DIR)/myaosp.mk\

COMMON_LUNCH_CHOICES :=\
        myaosp-userdebug\
    	myaosp-user\
    	myaosp-eng\

```

最后验证：

```bash
source build/envsetup.sh
lunch myaosp-eng
make -j16
emulator
```

## 参考资料

* [Android系统10 RK3399 init进程启动(十五) 配置新产品](https://blog.csdn.net/ldswfun/article/details/121931548?spm=1001.2014.3001.5502)
* [Android系统开发入门-2.添加product](http://qiushao.net/2019/11/19/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E5%85%A5%E9%97%A8/2-%E6%B7%BB%E5%8A%A0product/)
