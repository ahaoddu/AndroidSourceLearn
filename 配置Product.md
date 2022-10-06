## 配置 Product

## 什么是 Product

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


## Product 配置文件的位置与结构

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



我们主要关注一下几个文件：

* `/board/generic_x86_64/BoardConfig.mk` ： 用于配置开发板
* `/product/AndroidProducts.mk`   `/product/aosp_x86_64.mk`：用于配置 Product


我们自定义的 Product 一般放在 device 目录下，其结构一般如下：

![](https://gitee.com/stingerzou/pic-bed/raw/master/20221004192559.png)




## 添加 Product



## 参考资料
