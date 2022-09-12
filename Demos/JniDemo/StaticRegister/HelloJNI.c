#include "HelloJNI.h"
#include <stdio.h>
#include <jni.h>

JNIEXPORT void JNICALL Java_HelloJNI_sayHello(JNIEnv *env, jobject obj)
{
    printf("hello JNI\n");
    return;
}