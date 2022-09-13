# Android Framework 源码分析与实践

## 引子

现在是 2022 年 9 月 12 号，在成都，我已经不记得这是全城静态管理的第几天了。2021上半年还是一片欣欣向荣，下半年一开始，就能明显感觉到市场的寒气，业务锐减，营收锐减。春节一过，手机上就不停推送着大厂们裁员的消息，市场一片哀嚎。

Android 开发在知乎上已经“凉”了好几年了，学习 Android 的年轻人也越来越少了，35岁的达摩克利斯之剑在头顶摇摇晃晃地盯着我们。对于我们这些 Android 老人应该怎么办呢？应对的方法其实也很简单：

* 抓住风口，创业成功，豪车豪宅，迎娶白富美，走上巅峰
* 贵人提携，闷声发大财
* 中几个小目标的彩票，从此隐姓埋名，做一名快乐的富豪
* 有个好爹，回家继承上市公司家业

上面的场景一般只会出现在电影和梦里（继承家业的我认识好几个，上学的时候还在疑问？为什么他们只想考60分？还是太年轻，现在才知道能考 60 分的已经非常上进了），现实一点，2022 年了，App 开发已经很难了，老板们还需要你会：

* 音视频编解码、图形渲染，OpenGLES，OpenSLES，mediacodec，ffmpeg，librtmp，webrtc等等
* framework开发，安卓系统定制、车载系统、机器人、机顶盒等等
* 游戏开发，cocos2dx、unity3d、unreal4 等等游戏引擎
* 安卓安全逆向
* 跨平台，RN、flutter编写UI，ios要不要横向拓展下？
* 其他进阶方向 ......

熟练掌握了 App 开发，再掌握上述的一到两项技能，妥妥的技术大佬！

我认为，上述的多种技术，对于做 Android App 的开发人员，学习 framework 性价比是最高的，为什么这么说？熟悉 framework 可以让你的 App 开发更加地游刃有余，不想做 App了，还可以参与系统开发相关工作。学了不愁没有用。而且容易预见到，未来很多年里，新能源汽车飞速地发展，对于 framework 相关人才的需求量巨大。总之学 framework 血赚。

那么问题来了，网络上 Android 源码分析相关的文章以及市面上的书籍，要么是源码流程的分析，要么是高深原理的讲述。非常烧脑，学习起来挫败感很强。本教程从实践出发尽可能地做到了：

* **能看懂**
* **能上手**
* **能用于工作实践的**


## 计划

以下是教程的目录，陆续更新中：

[1.Android源码分析与实践-环境准备与源码下载](https://github.com/dducd/AndroidSourceLearn/blob/main/1.Android%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90%E4%B8%8E%E5%AE%9E%E8%B7%B5-%E7%8E%AF%E5%A2%83%E5%87%86%E5%A4%87%E4%B8%8E%E6%BA%90%E7%A0%81%E4%B8%8B%E8%BD%BD.md)

[2. 预备知识-如何在Android平台执行C/C++程序](https://github.com/dducd/AndroidSourceLearn/blob/main/%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86/%E9%A2%84%E5%A4%87%E7%9F%A5%E8%AF%86-%E5%A6%82%E4%BD%95%E5%9C%A8Android%E5%B9%B3%E5%8F%B0%E6%89%A7%E8%A1%8CC%20C%2B%2B%E7%A8%8B%E5%BA%8F.md)

[3.Linux驱动入门-模块](https://github.com/dducd/AndroidSourceLearn/blob/main/2.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E6%A8%A1%E5%9D%97.md)

[4.Linux驱动入门-驱动](https://github.com/dducd/AndroidSourceLearn/blob/main/3.Linux%E9%A9%B1%E5%8A%A8%E5%85%A5%E9%97%A8-%E9%A9%B1%E5%8A%A8.md)

5.预备知识-JNI入门


下图是计划中的路线图，陆续更新中，后续根据实际情况会做一些微调。

![](https://gitee.com/stingerzou/pic-bed/raw/master/AndroidFramework源码分析与实践.png)

## 源码下载

教程的源码都在 github 仓库的 [demos](https://github.com/ahaoddu/AndroidSourceLearn/tree/main/Demos) 目录下。

## 关于我

- 我叫阿豪，目前定居成都
- 2012 年开始从事 Android 系统定制和应用开发相关的工作
- 2015 年毕业于国防科技大学，毕业后从事 Android 相关的开发和研究工作
- 2019年初开始创业，从事 Android 系统开发工作
- 如果你对 Android 系统源码感兴趣可以扫码添加我的微信，相互学习交流。

![27c7e38ee991b9d1fb42cb3bdf352a7.jpg](https://cdn.nlark.com/yuque/0/2022/jpeg/2613680/1662174041146-53015bfc-12f7-4023-9131-0a9e51fd00a2.jpeg#clientId=u0593d637-e239-4&crop=0&crop=0&crop=1&crop=1&from=drop&id=ud527bf55&margin=%5Bobject%20Object%5D&name=27c7e38ee991b9d1fb42cb3bdf352a7.jpg&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&size=42506&status=done&style=none&taskId=uf620381e-5767-4559-867e-093d91d3256&title=#crop=0&crop=0&crop=1&crop=1&id=qxLzV&originHeight=430&originWidth=430&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)
