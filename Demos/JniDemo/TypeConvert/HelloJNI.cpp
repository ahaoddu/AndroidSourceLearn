#include "HelloJNI.h"
#include <stdio.h>
#include <jni.h>
#include <iostream>

using namespace std;

JNIEXPORT void JNICALL Java_HelloJNI_sayHello__(JNIEnv *env, jobject obj)
{
    printf("hello JNI\n");
    return;
}

/**
 *  传入的类型为 jstring
 */
JNIEXPORT jstring JNICALL Java_HelloJNI_sayHello__Ljava_lang_String_2(JNIEnv *env, jobject jobj, jstring str) {
    
    //jstring -> char*
    jboolean isCopy;
    const char* cStr = env->GetStringUTFChars(str, &isCopy);
    
    if (nullptr == cStr) {
        return nullptr;
    }

    if (JNI_TRUE == isCopy) {
        cout << "C 字符串是java字符串的一份拷贝" << endl;
    } else {
        cout << "C 字符串指向 java 层的字符串" << endl;
    }

    cout << "C/C++ 层接收到的字符串是 " << cStr << endl;
    
    //通过JNI GetStringChars 函数和 GetStringUTFChars 函数获得的C字符串在原生代码中
    //使用完之后需要正确地释放，否则将会引起内存泄露。
    env->ReleaseStringUTFChars(str, cStr);

    string outString = "Hello, JNI";
    // char* 转换为 string
    return env->NewStringUTF(outString.c_str());
}


JNIEXPORT jdouble JNICALL Java_HelloJNI_average(JNIEnv *env, jobject jobj, jint n1, jint n2) {
    //原始类型不用做转换，直接使用
    cout << "n1 = " << n1 << ", n2 = " << n2 << endl;
    return jdouble(n1 + n2)/2.0;
}


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

    //构造返回数据
    jdouble outArray[] = {sum, average};
    jdoubleArray outJNIArray = env->NewDoubleArray(2);
    if(NULL == outJNIArray) return NULL;
    //向 jdoubleArray 写入数据
    env->SetDoubleArrayRegion(outJNIArray, 0, 2, outArray);
    return outJNIArray;
}

JNIEXPORT void JNICALL Java_HelloJNI_modifyVariable(JNIEnv *env, jobject obj) {
    //0.获取到 jcalss
    jclass thisClass = env->GetObjectClass(obj);
    
    //1.获取到 filed 的 ID
    jfieldID numberId = env->GetFieldID(thisClass, "number", "I");
    if (nullptr == numberId) {
        return;
    }
    //2.根据 id 获取到 filed 
    jint number = env->GetIntField(obj, numberId);
    cout << "In C++, the number from java is " << number << endl;

    //3.给 filed 赋值
    number = 99;
    env->SetIntField(obj, numberId, number);

    //获取一个 String 成员变量，与获取 number 类似
    jfieldID messageId = env->GetFieldID(thisClass, "message", "Ljava/lang/String;");
    if(NULL == messageId) return;

    jstring message =  (jstring)env->GetObjectField(obj, messageId);

    const char* str = env->GetStringUTFChars(message, NULL);
    if(NULL == str) return;

    cout << "In C++, the message is " << str << endl;
    //内存清理
    env->ReleaseStringUTFChars(message, str);

    message = env->NewStringUTF("Hello from C++");
    if(NULL == message) return;

    env->SetObjectField(obj, messageId, message);

    //访问静态变量
    jfieldID staticNumberId = env->GetStaticFieldID(thisClass, "numberStatic", "D");
    if (nullptr == staticNumberId) {
        return;
    }
    jdouble number2 = env->GetStaticDoubleField(thisClass, staticNumberId);
    cout << "In C++, the static double is " << number2 << endl;

    number2 = 77.88;
    env->SetStaticDoubleField(thisClass, fidStaticNumber, number2);

}


// JNIEXPORT void JNICALL Java_HelloJNI_callJavaMethod(JNIEnv *env, jobject obj) {
//     //获取 class
//     jclass thisClass = env->GetObjectClass(obj);
//     //调用实例方法
//     jmethodID javaMethodId = env->GetMethodID(thisClass, "callJavaMethod","()V");
//     if (NULL == javaMethodId) {
//         return;
//     }
//     //方法名中的void表示，调用的 java 方法的返回值类型是 void
//     env->CallVoidMethod(obj, javaMethodId);
    
//     jmethodID javaStaticMethodId = env->GetStaticMethodID(thisClass, "callJavaMethod","()V");
//     if (NULL == javaStaticMethodId) {
//         return;
//     } 

// }
