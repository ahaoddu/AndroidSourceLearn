# JNI数据类型与描述符

## 1. 数据类型

JNI 程序中涉及了三种数据类型，分别是：

* Java 类型
* JNI 类型
* C/C++ 类型

在 java 程序中我们使用的是 Java 类型，C/C++ 程序中拿到的是 JNI 类型，我们需要将其转换为 C/C++ 类型，使用 C/C++ 类型再去调用 C/C++ 层函数完成计算或IO操作等任务后，将结果再转换为 JNI 类型返回后，在 java 代码中，我们就能收到对应的 Java 类型。

我们可以在 $JAVA_HOME/inlcude/jni.h 文件中查看到 jni 中基本类型的定义：

```c
typedef unsigned char   jboolean;
typedef unsigned short  jchar;
typedef short           jshort;
typedef float           jfloat;
typedef double          jdouble;
typedef jint            jsize;
```

jbyte, jint and jlong 是和 CPU 平台相关的，定义在 jni_md.h 中：

$JAVA_HOME/include/linux/jni_md.h

```c
typedef int jint;
#ifdef _LP64
typedef long jlong;
#else
typedef long long jlong;
#endif

typedef signed char jbyte;
```

x86_64 平台梳理如下：

| Java 类型 | JNI 类型 | C/C++ 类型      | 类型描述 |
| --------- | -------- | -------------- | ------ |
| boolean   | jboolean | unsigned char  | unsigned 8 bits |
| byte      | jbyte    | signed char    | signed 8 bits     |
| char      | jchar    | unsigned short | unsigned 16 bits     |
| short     | jshort   | signed short   | signed 16 bits     |
| int       | jint     | int            |  signed 32 bits     |
| long      | jlong    | long           |   signed 64 bits    |
| float     | jfloat   | float          |   32 bits    |
| double    | jdouble  | double         |   64 bits    |

这些数据类型可以直接使用无需转化:

```c++
jbyte result=0xff;
jint size;
jbyte* timeBytes;
```


引用类型也定义在 jni.h 中，总结如下：

| java 类型           | JNI 引用类型  |   类型描述 |
| ------------------- | ------------- | -------- |
| java.lang.Object    | jobject       | 可以表示任何Java的对象,（实例方法的强制参数）|
| java.lang.String    | jstring       | Java的String字符串类型的对象 |
| java.lang.Class     | jclass        | （实例方法的强制参数） |
| java.lang.Throwable | jthrowable    |  Java的Throwable类型，表示异常的所有类型和子类 |
| byte[]              | jbyteArray    | Java byte型数组 |
| Object[]            | jobjectArray  | Java任何对象的数组 |
| boolean[]           | jbooleanArray | Java boolean型数组 |
| char[]              | jcharArray    | Java char型数组 |
| short[]             | jshortArray   | Java short型数组 |
| int[]               | jintArray     | Java int型数组 |
| long[]              | jlongArray    | Java long型数组 |
| float[]             | jfloatArray   | Java float型数组 |
| double[]            | jdoubleArray  | Java double型数组 |
| void                | void          |         |

引用类型不能直接使用，要经过JNI函数转换才能使用，具体如何转换我们会在后面的章节介绍。


## 2. 描述符

所谓描述符，就是 C/C++ 层对 Java 世界一切的符号表示。

### 2.1 类描述符

假设这么一个场景，在 JNI 的 Native 方法中，我们要使用 Java 中的对象怎么办? 即在 C/C++ 中怎么找到 Java 中的类，这就要使用到 JNI 开发中的类描述符了。JNI 提供的函数中有个 FindClass() 就是用来查找 Java 类的，其参数必须放入一个类描述符字符串，类描述符一般是类的完整名称（包名+类名）。这么说是不是很抽象，下面看个例子：

完整类名:   java.lang.String
对应类描述符: java/lang/String

```c++
jclass intArrCls = env->FindClass(“java/lang/String”)
//在实践中，我发现可以直接用该类型的域描述符取代，也是可以成功的
jclass intArrCls = env->FindClass(“Ljava/lang/String;”)
```

### 2.2 域描述符

域描述符是 JNI 中对 Java 数据类型的一种表示方法。即在JVM虚拟机中，存储数据类型的名称时，是使用指定的描述符来存储，而不是我们习惯的 int，float 等。虽然有类描述符，但是类描述符里并没有说明基本类型和引用数据类型如何表示，所以在JNI中就引入了域描述符的概念。

基本类型域描述符：

| Java类型 | JNI 域描述符 |
| ----------------- | ----------------- |
|  int              | I                 |
| long | J |
| byte | B |
| short | S |
| char | C |
| double | D |
| boolean | Z |
| void | V | 


引用类型域描述符:

一般引用类型则为 `L + 该类型类描述符 + ;`:

| Java类型 | JNI 域描述符 |
| ----------------- | ----------------- |
| java.lang.String  | Ljava/lang/String; |

数组引用类型域描述符:

| Java类型 | JNI 域描述符 |
| ----------------- | ----------------- |
| int[] | [I |
| float[] | [F |
| String[] | [Ljava/lang/String; |
| Object[] | [Ljava/lang/Object; |
| int[][] | [[I |
| float[][] | [[F | 

一些示例代码：

```c++
get_field(env, javaBean_clazz, "boolValue", "Z", &javaBean_t.boolValue);
get_field(env, javaBean_clazz, "charValue", "C", &javaBean_t.charValue);
get_field(env, javaBean_clazz, "doubleValue", "D", &javaBean_t.doubleValue);
get_field(env, javaBean_clazz, "intValue", "I", &javaBean_t.intVaule);
get_field(env, javaBean_clazz, "array", "[B", &javaBean_t.byteArray);
get_field(env, javaBean_clazz, "mDoubleDimenArray", "[[I", &javaBean_t.double_dimen_array);
get_field(env, javaBean_clazz, "stringValue", "Ljava/lang/String;", &javaBean_t.stringValue);
get_field(env, javaBean_clazz, "mInnerClass", "Lcom/xxx/object2struct/JavaBean$InnerClass;", &javaBean_t.inner_message);
```


### 2.3 方法描述符

方法描述符定义了方法的返回值和参数的表示形式，将参数类型的域描述符按声明顺序放入一对括号中(如果没有参数则不需要括号)，括号后跟返回值类型的域描述符即形成方法描述符。概念比较抽象，我们看几个示例就明白了：

| Java 方法 | 方法描述符  |
| --------  | ---------  |
|  String fun() | ()Ljava/lang/String; |
| int fun(int i, Object object) | (ILjava/lang/Object;)I |
| void fun(byte[ ] bytes) | ([B)V |
| int fun(byte data1, byte data2) | (BB)I |
| void fun() | ()V | 

void 的处理比较特殊，如果返回值为 void，那么方法描述符中必须使用 V 表示，当 void 作为参数的时候忽略。

## 参考资料：

* [JNI/NDK开发指南（十）——JNI局部引用、全局引用和弱全局引用](https://blog.csdn.net/xyang81/article/details/44657385)
* [Android JNI 中的引用管理](https://glumes.com/post/android/android-jni-reference-manage-rules/)
* the java native interface programmer's guide and specification
* 《深入理解 Android Java 虚拟机 ART》 第11章第4节
* [JNI/NDK入门指南之JNI数据类型，描述符详解](https://blog.csdn.net/tkwxty/article/details/103526316)
