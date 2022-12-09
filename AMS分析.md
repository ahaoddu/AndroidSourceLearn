# AMS 分析

## 发起端进程请求启动目标 Activity

```java
//发起端进程发起启动Activity请求
	Activity.startActivity(...)
	  Activity.startActivityForResult(...)
	    mInstrumentation.execStartActivity(...)
		  AMP.startActivity(...)//通过AMS代理端向AMS发起启动Activity的Binder IPC请求
		  mInstrumentation.checkStartActivityResult(...)//检测启动是否成功
		mMainThread.sendActivityResult(...)
```
