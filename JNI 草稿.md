## 2. 数据类型转换


### 2.1 基本类型

基本类型无需做转换，直接使用：

java 层：

```java
private native double average(int n1, int n2);
```

c/c++ 层：

```c++
JNIEXPORT jdouble JNICALL Java_HelloJNI_average(JNIEnv *env, jobject jobj, jint n1, jint n2) {
    //原始类型不用做转换，直接使用
    cout << "n1 = " << n1 << ", n2 = " << n2 << endl;
    return jdouble(n1 + n2)/2.0;
}
```

### 2.2 字符串

为了在 C/C++ 中使用 Java 字符串，需要先将 Java 字符串转换成 C 字符串。用 GetStringChars 函数可以将 Unicode 格式的 Java 字符串转换成 C 字符串，用 GetStringUTFChars 函数可以将 UTF-8 格式的 Java 字符串转换成 C 字符串。这些函数的第三个参数均为 isCopy，它让调用者确定返回的 C 字符串地址指向副本还是指向堆中的固定对象。

java 层：

```java
private native String sayHello(String msg);
```

c/c++ 层：

```c++
jJNIEXPORT jstring JNICALL Java_HelloJNI_sayHello__Ljava_lang_String_2(JNIEnv *env, jobject jobj, jstring str) {
  
    //jstring -> char*
    jboolean isCopy;
    //GetStringChars 用于 unicode 编码
    //GetStringUTFChars 用于 utf-8 编码
    const char* cStr = env->GetStringUTFChars(str, &isCopy);
  
    if (nullptr == cStr) {
        return nullptr;
    }

    if (JNI_TRUE == isCopy) {
        cout << "C 字符串是 java 字符串的一份拷贝" << endl;
    } else {
        cout << "C 字符串指向 java 层的字符串" << endl;
    }

    cout << "C/C++ 层接收到的字符串是 " << inStr << endl;
  
    //通过JNI GetStringChars 函数和 GetStringUTFChars 函数获得的C字符串在原生代码中
    //使用完之后需要正确地释放，否则将会引起内存泄露。
    env->ReleaseStringUTFChars(str, inStr);

    string outString = "Hello, JNI";
    // char* 转换为 string
    return env->NewStringUTF(outString.c_str());
}
```

### 2.3 数组

java 层：

```java
private native double[] sumAndAverage(int[] numbers);
```

c++ 层：

```cpp
JNIEXPORT jdoubleArray JNICALL Java_HelloJNI_sumAndAverage(JNIEnv *env, jobject obj, jintArray inJNIArray) {
    //类型转换 int[] -> jintArray -> jint*
    jboolean isCopy;
    jint* inArray = env->GetIntArrayElements(inJNIArray, &isCopy);

    if (JNI_TRUE == isCopy) {
        cout << "C 层的数组是 java 层数组的一份拷贝" << endl;
    } else {
        cout << "C 层的数组指向 java 层的数组" << endl;
    }

    if(nullptr == inArray) return nullptr;
    //获取到数组长度
    jsize length = env->GetArrayLength(inJNIArray);

    jint sum = 0;
    for(int i = 0; i < length; ++i) {
        sum += inArray[i];
    }

    jdouble average = (jdouble)sum / length;
    //释放数组
    env->ReleaseIntArrayElements(inJNIArray, inArray, 0); // release resource

    //构造返回数据，outArray 是指针类型，需要 free 或者 delete 吗？要的
    jdouble outArray[] = {sum, average};
    jdoubleArray outJNIArray = env->NewDoubleArray(2);
    if(NULL == outJNIArray) return NULL;
    //向 jdoubleArray 写入数据
    env->SetDoubleArrayRegion(outJNIArray, 0, 2, outArray);
    return outJNIArray;
}
```