## 局部与全局引用


## 异常处理

## 多线程

## NIO 操作

## JNI 与 NULL

### 3.3 示例

本节的示例主要展示 java 和 c 层之间数据类型的转换与方法访问，用于开发时的参考。

##### 3.3.1 原始类型

```java

//java
private native double average(int n1, int n2);

//c++
JNIEXPORT jdouble JNICALL Java_TestJNIPrimitive_average(
    JNIEnv *env, jobject obj, jint n1, jint n2)
{
    //原始类型不用做转换，直接使用
    cout << "n1 = " << n1 << ", n2 = " << n2 << endl;
    return jdouble(n1 + n2)/2.0;
}
```

##### 3.2.2 字符串

```java
//java
private native String sayHello(String msg);

//c++
JNIEXPORT jstring JNICALL Java_TestJNIString_sayHello(
    JNIEnv *env, jobject obj, jstring inJNIString)
{
    //将 java 中的 String 转换为 char*
    const char* inStr = env->GetStringUTFChars(inJNIString, NULL);
    if(NULL == inStr)
        return NULL;

    //内存清理工作
    cout << "the received string is " << inStr << endl;
    env->ReleaseStringUTFChars(inJNIString, inStr);

    string outString;
    cout << "Enter a String:";
    cin >> outString;
    // char* 转换为 string
    return env->NewStringUTF(outString.c_str());
}
```

##### 3.2.3 数组

```java
 //java
// 返回数组double[2]，其中double[0]为和，double[1]为平均数
private native double[] sumAndAverage(int[] numbers);

//c++
JNIEXPORT jdoubleArray JNICALL Java_TestJNIPrimitiveArray_sumAndAverage(
    JNIEnv *env, jobject obj, jintArray inJNIArray)
{
    //int[] -> jintArray -> jint*
    jint* inArray = env->GetIntArrayElements(inJNIArray, NULL);
    if(NULL == inArray) return NULL;
    //获取到数组长度
    jsize length = env->GetArrayLength(inJNIArray);

    jint sum = 0;
    for(int i = 0; i < length; ++i)
    {
        sum += inArray[i];
    }

    jdouble average = (jdouble)sum / length;
    //释放数组
    env->ReleaseIntArrayElements(inJNIArray, inArray, 0); // release resource

    jdouble outArray[] = {sum, average};
    jdoubleArray outJNIArray = env->NewDoubleArray(2);
    if(NULL == outJNIArray) return NULL;
    env->SetDoubleArrayRegion(outJNIArray, 0, 2, outArray);
    return outJNIArray;

}
```

##### 3.2.4 访问对象的实例变量

```java
//java
public class TestJNIInstanceVariable{
    static {
        System.loadLibrary("myjni");
    }

    private int number = 88;
    private String message = "Hello from Java";

    private native void modifyInstanceVariable();

    public static void main(String[] args){
        TestJNIInstanceVariable test = new TestJNIInstanceVariable();
        test.modifyInstanceVariable();

        System.out.println("In Java, int is " + test.number);
        System.out.println("In Java, String is " + test.message);
    }
}
```

```c
//c++
#include "TestJNIInstanceVariable.h"
#include <iostream>

using namespace std;

JNIEXPORT void JNICALL Java_TestJNIInstanceVariable_modifyInstanceVariable(
    JNIEnv *env, jobject thisObj)
{
    // Get a reference to this object's class
    jclass thisClass = env->GetObjectClass(thisObj);

    jfieldID fidNumber = env->GetFieldID(thisClass, "number", "I");
    if(NULL == fidNumber) return;

    // Get the int given the Field ID
    jint number = env->GetIntField(thisObj, fidNumber);
    cout << "In C++, the int is " << number << endl;

    // Change the variable
    number = 99;
    env->SetIntField(thisObj, fidNumber, number);

    // String
    // Get the Field ID of the instance variables "message"
    jfieldID fidMessage = env->GetFieldID(thisClass, "message", "Ljava/lang/String;");
    if(NULL == fidMessage) return;

    // Get the int given the Field ID
    jstring message =  (jstring)env->GetObjectField(thisObj, fidMessage);

    // Create a C-String with JNI String
    const char* str = env->GetStringUTFChars(message, NULL);
    if(NULL == str) return;

    cout << "In C++, the string is " << str << endl;

    // Create a new C-String and assign to the JNI string
    message = env->NewStringUTF("Hello from C++");
    if(NULL == message) return;

    env->SetObjectField(thisObj, fidMessage, message);
}

```

通过 GetFieldID 获取实例变量的字段 ID，这需要你提供变量的名称以及其字段的描述符：

| **描述符**   | **含义** |
| ------------------ | -------------- |
| I                  | int            |
| B                  | byte           |
| S                  | short          |
| J                  | long           |
| F                  | float          |
| D                  | double         |
| C                  | char           |
| Z                  | boolean        |
| [I                 | int[]          |
| Ljava/lang/String  | String         |
| [Ljava/lang/String | String[]       |

##### 3.2.5 访问类的静态变量

java 层：

```java
//java
public class TestJNIStaticVariable{
    static {
        System.loadLibrary("myjni");
    }

    private static double number = 55.66;

    private native void modifyStaticVariable();

    public static void main(String args[]) {
        TestJNIStaticVariable test = new TestJNIStaticVariable();
        test.modifyStaticVariable();
        System.out.println("In Java, the double is " + number);
    }
}
```

cpp 层：

```cpp
#include "TestJNIStaticVariable.h"
#include <iostream>
using namespace std;

JNIEXPORT void JNICALL Java_TestJNIStaticVariable_modifyStaticVariable(
    JNIEnv *env, jobject thisObj)
{
    jclass thisClass = env->GetObjectClass(thisObj);

    jfieldID fidNumber = env->GetStaticFieldID(thisClass, "number", "D");
    if(NULL == fidNumber) return;

    jdouble number = env->GetStaticDoubleField(thisClass, fidNumber);
    cout << "In C++, the double is " << number << endl;

    number = 77.88;
    env->SetStaticDoubleField(thisClass, fidNumber, number);
}
```

##### 3.2.6 回调实例方法和静态方法

```java
public class TestJNICallBackMethod{
    static {
        System.loadLibrary("myjni");
    }

    private native void nativeMethod();

    private void callback(){
        System.out.println("In Java");
    }

    private void callback(String message){
        System.out.println("In Java with " + message);
    }

    private double callbackAverage(int n1, int n2){
        return ((double)n1 + n2) / 2.0;
    }

    private static String callbackStatic(){
        return "From static Java method";
    }

    public static void main(String[] args){
        new TestJNICallBackMethod().nativeMethod();
    }
}
```

```cpp
#include "TestJNICallBackMethod.h"
#include <iostream>

using namespace std;

JNIEXPORT void JNICALL Java_TestJNICallBackMethod_nativeMethod(
    JNIEnv *env, jobject thisObj)
{
    jclass thisClass = env->GetObjectClass(thisObj);

    jmethodID midCallBack = env->GetMethodID(thisClass, "callback", "()V");
    if(NULL == midCallBack) return;
    cout << "In C++, call back Java's callback()\n";

    // Call back the method (which returns void), based on the Method ID
    env->CallVoidMethod(thisObj, midCallBack);

    jmethodID midCallBackStr = env->GetMethodID(thisClass, "callback", "(Ljava/lang/String;)V");
    if(NULL == midCallBackStr) return;
    cout << "In C++, call back Java's callback(String)\n";
    jstring message = env->NewStringUTF("Hello from C++");
    env->CallVoidMethod(thisObj, midCallBackStr, message);

    jmethodID midCallBackAverage = env->GetMethodID(thisClass, "callbackAverage", "(II)D");
    if(NULL == midCallBackAverage) return;
    jdouble average = env->CallDoubleMethod(thisObj, midCallBackAverage, 2, 3);
    cout << "In C++, the average is " << average << endl;

    jmethodID midCallBackStatic = env->GetStaticMethodID(thisClass, "callbackStatic", "()Ljava/lang/String;");
    if(NULL == midCallBackStatic) return;
    jstring resultJNIStr = (jstring)env->CallStaticObjectMethod(thisClass, midCallBackStatic);
    const char* resultStr = env->GetStringUTFChars(resultJNIStr, NULL);
    if(NULL == resultStr) return;
    cout << "In C++, the returned string is " << resultStr << endl;
    env->ReleaseStringUTFChars(resultJNIStr, resultStr);
}
```

##### 3.2.7 回调 super.xx() 方法

JNI 提供了一组 CallNonvirtual `<Type>`Method() 函数来调用在子类重写的方法（类型在 Java 中调用 super.methodName())

通过 GetMethodID() 获取方法ID
基于方法ID，调用 CallNonvirtual `<Type>`Method() 来回调父类方法
JNI 中用于调用重载的父类方法函数有：

```c
NativeType CallNonvirtual<type>Method(JNIEnv *env, jobject obj, jclass cls, jmethodID methodID, ...);
NativeType CallNonvirtual<type>MethodA(JNIEnv *env, jobject obj, jclass cls, jmethodID methodID, const jvalue *args);
NativeType CallNonvirtual<type>MethodV(JNIEnv *env, jobject obj, jclass cls, jmethodID methodID, va_list args);
```

##### 3.2.8 在 native 中创建对象

```java
public class TestJavaConstructor{
    static {
        System.loadLibrary("myjni");
    }

    private native Integer getIntegerObject(int number);

    public static void main(String[] args){
        TestJavaConstructor obj = new TestJavaConstructor();
        System.out.println("In Java, the number is : " + obj.getIntegerObject(9999));
    }
}
```

```c
#include "TestJavaConstructor.h"
#include <iostream>
using namespace std;

JNIEXPORT jobject JNICALL Java_TestJavaConstructor_getIntegerObject
    (JNIEnv *env, jobject thisObj, jint number)
{
    jclass cls = env->FindClass("java/lang/Integer");

    jmethodID midInit = env->GetMethodID(cls, "<init>", "(I)V");
    if(NULL == midInit) return NULL;

    // Call back constructor to allocate a new instance, with an int argument
    jobject newObj = env->NewObject(cls, midInit, number);

    // Try running the toString() on this newly create object
    jmethodID midToString = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
    if (NULL == midToString) return NULL;

    jstring resultJNIStr = (jstring)env->CallObjectMethod(newObj, midToString);
    const char *resultStr = env->GetStringUTFChars(resultJNIStr, NULL);
    cout << "In C++, the number is " << resultStr << endl;

    return newObj;
}
```

JNI 中用于创建对象的函数有：

```c
jclass FindClass(JNIEnv *env, const char *name);
 
jobject NewObject(JNIEnv *env, jclass cls, jmethodID methodID, ...);
jobject NewObjectA(JNIEnv *env, jclass cls, jmethodID methodID, const jvalue *args);
jobject NewObjectV(JNIEnv *env, jclass cls, jmethodID methodID, va_list args);
   // Constructs a new Java object. The method ID indicates which constructor method to invoke
 
jobject AllocObject(JNIEnv *env, jclass cls);
  // Allocates a new Java object without invoking any of the constructors for the object.

```

##### 3.2.9 对象数组

```java
import java.util.ArrayList;
 
public class TestJNIObjectArray {
   static {
      System.loadLibrary("myjni"); 
   }
   // Native method that receives an Integer[] and
   //  returns a Double[2] with [0] as sum and [1] as average
   private native Double[] sumAndAverage(Integer[] numbers);
 
   public static void main(String args[]) {
      Integer[] numbers = {11, 22, 32};  // auto-box
      Double[] results = new TestJNIObjectArray().sumAndAverage(numbers);
      System.out.println("In Java, the sum is " + results[0]);  // auto-unbox
      System.out.println("In Java, the average is " + results[1]);
   }
}
```

```cpp
#include "TestJNIObjectArray.h"
#include <iostream>
using namespace std;

JNIEXPORT jobjectArray JNICALL Java_TestJNIObjectArray_sumAndAverage(
    JNIEnv *env, jobject thisObj, jobjectArray inJNIArray)
{
    jclass classInteger = env->FindClass("java/lang/Integer");

    // Use Integer.intValue() to retrieve the int
    jmethodID midIntValue = env->GetMethodID(classInteger, "intValue", "()I");
    if (NULL == midIntValue) return NULL;

    jsize length = env->GetArrayLength(inJNIArray);
    jint sum = 0;
    for (int i = 0; i < length; i++) {
        jobject objInteger = env->GetObjectArrayElement(inJNIArray, i);
        if (NULL == objInteger) return NULL;
        jint value = env->CallIntMethod(objInteger, midIntValue);
        sum += value;
    }

    double average = (double)sum / length;
    cout << "In C++, the sum is " << sum << endl;
    cout << "In C++, the average is " << average << endl;

    // Get a class reference for java.lang.Double
    jclass classDouble = env->FindClass("java/lang/Double");

    // Allocate a jobjectArray of 2 java.lang.Double
    jobjectArray outJNIArray = env->NewObjectArray(2, classDouble, NULL);

    // Construct 2 Double objects by calling the constructor
    jmethodID midDoubleInit = env->GetMethodID(classDouble, "<init>", "(D)V");
    if (NULL == midDoubleInit) return NULL;
    jobject objSum = env->NewObject(classDouble, midDoubleInit, (double)sum);
    jobject objAve = env->NewObject(classDouble, midDoubleInit, average);

    // Set to the jobjectArray
    env->SetObjectArrayElement(outJNIArray, 0, objSum);
    env->SetObjectArrayElement(outJNIArray, 1, objAve);

    return outJNIArray;
}
```

JNI 中用于创建和操作对象数组的函数有：

```cpp
jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement);
   // Constructs a new array holding objects in class elementClass.
   // All elements are initially set to initialElement.
 
jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index);
   // Returns an element of an Object array.
 
void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value);
   // Sets an element of an Object array.
```

##### 3.2.10 局部和全局引用

在 native 方法中，我们经常使用 FindClass(), GetMethodID(), GetFieldID() 来获取 jclass, jmethodID 和 jfieldID。这些方法的调用成本很高，我们应该获取一次并且将其缓存以供后续使用，而不是重复执行调用，从而消除开销。

JNI 中 native 代码使用的对象引用分为两种：局部引用和全局引用：

1. 局部引用在 native 方法中创建，并在退出 native 方法时销毁。可以使用 DeleteLocalRef() 显示的使局部引用失效，以便可以进行垃圾回收。
2. 全局引用只有显示使用 DeleteGlobalRef() 才会被销毁。可以通过 NewGlobalRef() 从局部引用创建全局引用。

[
](https://blog.csdn.net/weiwei9363/article/details/97886291)

```java
public class TestJNIReference {
   static {
      System.loadLibrary("myjni"); // myjni.dll (Windows) or libmyjni.so (Unixes)
   }
 
   // A native method that returns a java.lang.Integer with the given int.
   private native Integer getIntegerObject(int number);
 
   // Another native method that also returns a java.lang.Integer with the given int.
   private native Integer anotherGetIntegerObject(int number);
 
   public static void main(String args[]) {
      TestJNIReference test = new TestJNIReference();
      System.out.println(test.getIntegerObject(1));
      System.out.println(test.getIntegerObject(2));
      System.out.println(test.anotherGetIntegerObject(11));
      System.out.println(test.anotherGetIntegerObject(12));
      System.out.println(test.getIntegerObject(3));
      System.out.println(test.anotherGetIntegerObject(13));
   }
}
```

上面的代码有两个 native 方法，它们都返回 java.lang.Integer 对象。

在 C/C++ 代码中，我们通过 FindClass() 需要获取 java.lang.Integer 的引用。然后找到 Integer 的构造函数ID。我们希望将这些都缓存起来以消除开销。

下面代码是不起作用的：

```cpp
#include <jni.h>
#include <stdio.h>
#include "TestJNIReference.h"
 
// Global Reference to the Java class "java.lang.Integer"
static jclass classInteger;
static jmethodID midIntegerInit;
 
jobject getInteger(JNIEnv *env, jobject thisObj, jint number) {
 
   // Get a class reference for java.lang.Integer if missing
   if (NULL == classInteger) {
      printf("Find java.lang.Integer\n");
      classInteger = (*env)->FindClass(env, "java/lang/Integer");
   }
   if (NULL == classInteger) return NULL;
 
   // Get the Method ID of the Integer's constructor if missing
   if (NULL == midIntegerInit) {
      printf("Get Method ID for java.lang.Integer's constructor\n");
      midIntegerInit = (*env)->GetMethodID(env, classInteger, "<init>", "(I)V");
   }
   if (NULL == midIntegerInit) return NULL;
 
   // Call back constructor to allocate a new instance, with an int argument
   jobject newObj = (*env)->NewObject(env, classInteger, midIntegerInit, number);
   printf("In C, constructed java.lang.Integer with number %d\n", number);
   return newObj;
}
 
JNIEXPORT jobject JNICALL Java_TestJNIReference_getIntegerObject
          (JNIEnv *env, jobject thisObj, jint number) {
   return getInteger(env, thisObj, number);
}
 
JNIEXPORT jobject JNICALL Java_TestJNIReference_anotherGetIntegerObject
          (JNIEnv *env, jobject thisObj, jint number) {
   return getInteger(env, thisObj, number);
}

```

上述代码中，我们调用 FindClass() 来获取 java.lang.Integer 的引用，并保存在全局的静态变量中。尽管如此，在下一次调用中，此引用不再有效（并且不是NULL）。这是因为FindClass（）返回一个本地引用，一旦该方法退出就会失效。

为了解决这个问题，我们需要从FindClass（）返回的局部引用创建一个全局引用。然后我们可以释放局部引用。修改后的代码如下：
[
](https://blog.csdn.net/weiwei9363/article/details/97886291)

```cpp
 // Get a class reference for java.lang.Integer if missing
   if (NULL == classInteger) {
      printf("Find java.lang.Integer\n");
      // FindClass returns a local reference
      jclass classIntegerLocal = (*env)->FindClass(env, "java/lang/Integer");
      // Create a global reference from the local reference
      classInteger = (*env)->NewGlobalRef(env, classIntegerLocal);
      // No longer need the local reference, free it!
      (*env)->DeleteLocalRef(env, classIntegerLocal);
   }
```
