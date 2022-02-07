//
// Created by aimar on 5/3/2020.
//

#ifndef TYPEFACE_H
#define TYPEFACE_H

#include <jni.h>

enum FontStyle
{
    NORMAL = 0,
    BOLD = 1,
    ITALIC = 2,
    BOLD_ITALIC = 3
};

class Typeface {
public:
    JNIEnv *env;
    jclass typefaceObj;

    jmethodID createMethod;

    Typeface(JNIEnv *env)
    {
        this->env = env;

        typefaceObj = env->FindClass("android/graphics/Typeface");
        createMethod = env->GetStaticMethodID(typefaceObj, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
    }

    jobject create(const char *family, int style);
};


#endif //PUBG_TYPEFACE_H
