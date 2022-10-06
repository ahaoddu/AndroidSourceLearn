# Android Framework 源码分析与实践

## 引子

## 计划

![](https://gitee.com/stingerzou/pic-bed/raw/master/AndroidFramework源码分析与实践.png)

## 开发环境

[Android源码分析与实践-环境准备与源码下载](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/Android%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90%E4%B8%8E%E5%AE%9E%E8%B7%B5-%E7%8E%AF%E5%A2%83%E5%87%86%E5%A4%87%E4%B8%8E%E6%BA%90%E7%A0%81%E4%B8%8B%E8%BD%BD.md)

## 预备知识

预备知识这块我会挑一些重要的和容易忽略的点讲讲，不要在全部掌握后再学习framework，应该在学习过程中查疑补缺：

[预备知识-如何在Android平台执行C/C++程序](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/2.%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86-%E5%A6%82%E4%BD%95%E5%9C%A8Android%E5%B9%B3%E5%8F%B0%E6%89%A7%E8%A1%8CC%20C%2B%2B%E7%A8%8B%E5%BA%8F.md)

[预备知识-JNI入门](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/3.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E6%A8%A1%E5%9D%97.md)

[预备知识-JNI数据类型](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/5.2.%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86-JNI%E6%95%B0%E6%8D%AE%E7%B1%BB%E5%9E%8B.md)

[预备知识-理解 C++ 的 Memory Order](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/blob/main/%E7%90%86%E8%A7%A3%20C%2B%2B%20%E7%9A%84%20Memory%20Order.md)

## Android 系统开发入门

[配置 Product](https://github.com/ahaoddu/AndroidKnowledgeHierarchy/blob/main/4.Framework%E5%BC%80%E5%8F%91/%E9%85%8D%E7%BD%AEProduct.md)

添加自定义模块

添加预定义模块

添加系统服务

添加hidl服务

## 给你的Android添加一个硬件访问服务

[Linux驱动入门-模块](https://github.com/dducd/AndroidSourceLearn/blob/main/2.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E6%A8%A1%E5%9D%97.md)

[Linux驱动入门-驱动](https://github.com/ahaoddu/AndroidSourceLearn/blob/main/4.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E9%A9%B1%E5%8A%A8.md)

硬件的直接访问

给系统添加一个硬件访问服务

## 启动过程

init

zygote

servicemanager

systemserver

启动Android App

## 源码

教程的源码都在 github 仓库的 [demos](https://github.com/ahaoddu/AndroidSourceLearn/tree/main/Demos) 目录下。

## 关于我

- 我叫阿豪，目前定居成都
- 2012 年开始从事 Android 系统定制和应用开发相关的工作
- 2015 年本科毕业，毕业后从事 Android 相关的开发和研究工作
- 2019年初开始创业，从事 Android 系统开发工作
- 如果你对 Android 系统源码感兴趣可以扫码添加我的微信，相互学习交流。

![27c7e38ee991b9d1fb42cb3bdf352a7.jpg](https://cdn.nlark.com/yuque/0/2022/jpeg/2613680/1662174041146-53015bfc-12f7-4023-9131-0a9e51fd00a2.jpeg#clientId=u0593d637-e239-4&crop=0&crop=0&crop=1&crop=1&from=drop&id=ud527bf55&margin=%5Bobject%20Object%5D&name=27c7e38ee991b9d1fb42cb3bdf352a7.jpg&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&size=42506&status=done&style=none&taskId=uf620381e-5767-4559-867e-093d91d3256&title=#crop=0&crop=0&crop=1&crop=1&id=qxLzV&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)
