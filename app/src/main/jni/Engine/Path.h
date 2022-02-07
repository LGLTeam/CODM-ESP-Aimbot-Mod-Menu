//
// Created by ASUS on 23/11/2020.
//

#ifndef PATH_H
#define PATH_H

#include <jni.h>

#include "Const.h"

class Path {
public:
    JNIEnv *env;
    jobject pathObj;

    jmethodID lineToId;
    jmethodID moveToId;
    jmethodID closeId;
    jmethodID resetId;

    Path(JNIEnv *env) {
        this->env = env;

        jclass pathClass = env->FindClass("android/graphics/Path");
        jmethodID init = env->GetMethodID(pathClass, "<init>", "()V");
        this->pathObj = env->NewGlobalRef(env->NewObject(pathClass, init));

        lineToId = env->GetMethodID(pathClass, "lineTo", "(FF)V");
        moveToId = env->GetMethodID(pathClass, "moveTo", "(FF)V");
        closeId = env->GetMethodID(pathClass, "close", "()V");
        resetId = env->GetMethodID(pathClass, "reset", "()V");

        env->DeleteLocalRef(pathClass);
    }

    void lineTo(float x, float y);
    void moveTo(float x, float y);
    void close();
    void reset();
};


#endif //PUBG_PATH_H
