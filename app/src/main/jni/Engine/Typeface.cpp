//
// Created by aimar on 5/3/2020.
//

#include "Typeface.h"

jobject Typeface::create(const char *family, int style) {
    jstring fam = env->NewStringUTF(family);

    jobject result = env->CallStaticObjectMethod(this->typefaceObj, this->createMethod, fam, style);

    env->DeleteLocalRef(fam);

    return result;
}