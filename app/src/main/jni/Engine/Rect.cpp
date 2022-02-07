//
// Created by aimar on 1/13/2020.
//

#include "Rect.h"

int Rect::getLeft() {
    return env->GetIntField(this->rectObj, this->leftId);
}

int Rect::getRight(){
    return env->GetIntField(this->rectObj, this->rightId);
}

int Rect::getTop(){
    return env->GetIntField(this->rectObj, this->topId);
}

int Rect::getBottom(){
    return env->GetIntField(this->rectObj, this->bottomId);
}

int Rect::getWidth() {
    return getRight() - getLeft();
}

int Rect::getHeight() {
    return getBottom() - getTop();
}