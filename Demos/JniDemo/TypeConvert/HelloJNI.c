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


JNIEXPORT jstring JNICALL Java_HelloJNI_sayHello__Ljava_lang_String_2(JNIEnv *env, jobject jobj, jstring str) {
    //jstring -> char*
    const char* inStr = env->GetStringUTFChars(str, NULL);
    if (nullptr == inStr) {
        return nullptr;
    }
    
    //清理内存
    cout << "the received string is " << inStr << endl;
    env->ReleaseStringUTFChars(str, inStr);

    string outString;
    cout << "Enter a String:";
    cin >> outString;
    // char* 转换为 string
    return env->NewStringUTF(outString.c_str());
}


JNIEXPORT jdouble JNICALL Java_HelloJNI_average(JNIEnv *env, jobject jobj, jint n1, jint n2) {
    //原始类型不用做转换，直接使用
    cout << "n1 = " << n1 << ", n2 = " << n2 << endl;
    return jdouble(n1 + n2)/2.0;
}


JNIEXPORT jdoubleArray JNICALL Java_HelloJNI_sumAndAverage(JNIEnv *env, jobject obj, jintArray inJNIArray) {
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