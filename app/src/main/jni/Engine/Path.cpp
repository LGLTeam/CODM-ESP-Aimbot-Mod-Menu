//
// Created by ASUS on 23/11/2020.
//

#include "Path.h"

void Path::lineTo(float x, float y) {
    return env->CallVoidMethod(this->pathObj, this->lineToId, x, y);
}

void Path::moveTo(float x, float y) {
    return env->CallVoidMethod(this->pathObj, this->moveToId, x, y);
}

void Path::close() {
    return env->CallVoidMethod(this->pathObj, this->closeId);
}

void Path::reset() {
    return env->CallVoidMethod(this->pathObj, this->resetId);
}