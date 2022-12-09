# 添加自定义模块之 Android Studio 项目

这里演示一下怎么把 Android Studio 创建的项目导入到 Android 源码中。

使用 Android Studio 新建一个空项目 FirstSystemApp，java kotlin 都可以。创建完成后，将项目移动到 aosp/device/mycompamy 目录下。在项目的根目录下添加 Android.mk 文件：

```makefile
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,  app/src/main/java)

LOCAL_PACKAGE_NAME := FirstSystemApp

LOCAL_SDK_VERSION := current

#LOCAL_PROGUARD_FLAG_FILES := proguard.flags
# 签名方式
LOCAL_CERTIFICATE := platform

LOCAL_USE_AAPT2 := true


# 指定Manifest文件
LOCAL_MANIFEST_FILE := app/src/main/AndroidManifest.xml

LOCAL_RESOURCE_DIR := \
	$(addprefix $(LOCAL_PATH)/, app/src/main/res) 

# 项目依赖
LOCAL_STATIC_JAVA_LIBRARIES := \
	androidx.appcompat_appcompat  \
	androidx.recyclerview_recyclerview \
	com.google.android.material_material \
	androidx-constraintlayout_constraintlayout \


include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

```

修改 AndroidManifest.xml 文件：

```makefile
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:tools="http://schemas.android.com/tools"
    package="com.ahaoddu.firstsystemapp">

    <application
        android:allowBackup="true"
        android:fullBackupContent="@xml/backup_rules"
        android:icon="@mipmap/ic_launcher"
        android:label="@string/app_name"
        android:roundIcon="@mipmap/ic_launcher_round"
        android:supportsRtl="true"
        android:theme="@style/Theme.FirstSystemApp"
        tools:targetApi="31">
        <activity
            android:name=".MainActivity"
            android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />

                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>

            <meta-data
                android:name="android.app.lib_name"
                android:value="" />
        </activity>
    </application>

</manifest>
```

主要修改了两个地方：

* manifest 节点添加 package 属性
* application 节点把 android:dataExtractionRules 这一行删掉。使用最新的 Android Studio 创建项目会有这个，貌似在 Android10 源码中不支持

通常开发 app 需要使用到一些库，使用 find 或者 grep 在 prebuild 下搜索，搜不到再自己导入。

当然也可以通过添加 Android.bp 来编译项目：

```makefile
android_app {
    name: "FirstSystemApp",

    srcs: ["app/src/main/java/**/*.java"],

    sdk_version: "current",

    //LOCAL_PROGUARD_FLAG_FILES := proguard.flags
    certificate: "platform",

    // 指定Manifest文件
    manifest: "app/src/main/AndroidManifest.xml",

    resource_dirs: ["app/src/main/res"],

    static_libs: ["androidx.appcompat_appcompat",
                  "androidx.recyclerview_recyclerview",
                 "com.google.android.material_material",
                 "androidx-constraintlayout_constraintlayout"],

}
```

编译方式都是一样的：

```makefile
mm
```

常用的自定义模块：

* C/C++ 可执行程序
* C/C++ 库
* Java库
* Android 库
* apk

已经介绍完了，如果要写一些没有提到的模块可以在源码里面搜索（find grep），参考源码编写即可。