//
// Created by aimar on 1/13/2020.
//

#include "Paint.h"
#include "Canvas.h"
#include <wchar.h>

jstring wcstojstr2(JNIEnv *env, const wchar_t *input) {
    jobject bb = env->NewDirectByteBuffer((void *) input, wcslen(input) * sizeof(wchar_t));
    static jstring UTF32LE = 0;
    if (!UTF32LE)
        UTF32LE = (jstring) env->NewGlobalRef(env->NewStringUTF("UTF-32LE"));

    static jclass charsetClass = 0;
    if (!charsetClass)
        charsetClass = env->FindClass("java/nio/charset/Charset");

    static jmethodID forNameMethod = 0;
    if (!forNameMethod)
        forNameMethod = env->GetStaticMethodID(charsetClass, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");

    static jobject charset = 0;
    if (!charset)
        charset = env->NewGlobalRef(env->CallStaticObjectMethod(charsetClass, forNameMethod, UTF32LE));

    static jmethodID decodeMethod = 0;
    if (!decodeMethod)
        decodeMethod = env->GetMethodID(charsetClass, "decode", "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;");

    jobject cb = env->CallObjectMethod(charset, decodeMethod, bb);

    static jclass charBufferClass = 0;
    if (!charBufferClass)
        charBufferClass = env->FindClass("java/nio/CharBuffer");

    static jmethodID toStringMethod = 0;
    if (!toStringMethod)
        toStringMethod = env->GetMethodID(charBufferClass, "toString", "()Ljava/lang/String;");

    auto ret = (jstring) env->CallObjectMethod(cb, toStringMethod);

    env->DeleteLocalRef(bb);
    env->DeleteLocalRef(cb);

    return ret;
}

void Paint::setTextSize(float size) {
    env->CallVoidMethod(this->paintObj, this->setTextSizeId, size);
}

void Paint::setColor(int color) {
    env->CallVoidMethod(this->paintObj, this->setColorId, color);
}

void Paint::setStyle(Style style) {
    if (style == Style::FILL)
        env->CallVoidMethod(this->paintObj, this->setStyleId, this->Style_FILL);
    if (style == Style::STROKE)
        env->CallVoidMethod(this->paintObj, this->setStyleId, this->Style_STROKE);
    if (style == Style::FILL_AND_STROKE)
        env->CallVoidMethod(this->paintObj, this->setStyleId, this->Style_FILL_AND_STROKE);
}

Rect *Paint::getTextBounds(const char *text, int start, int end) {
    jstring str = env->NewStringUTF(text);
    env->CallVoidMethod(this->paintObj, this->getTextBoundsId, str, start, end, m_Rect->rectObj);
    env->DeleteLocalRef(str);
    return m_Rect;
}

Rect *Paint::getTextBounds(const wchar_t *text, int start, int end) {
    jstring str = wcstojstr2(this->env, text);
    env->CallVoidMethod(this->paintObj, this->getTextBoundsId, str, start, end, m_Rect->rectObj);
    env->DeleteLocalRef(str);
    return m_Rect;
}

void Paint::setStrokeWidth(float size) {
    env->CallVoidMethod(this->paintObj, this->setStrokeWidthId, size);
}

void Paint::setTextAlign(Align align) {
    if (align == Align::LEFT)
        env->CallVoidMethod(this->paintObj, this->setTextAlignId, this->Align_LEFT);
    if (align == Align::RIGHT)
        env->CallVoidMethod(this->paintObj, this->setTextAlignId, this->Align_RIGHT);
    if (align == Align::CENTER)
        env->CallVoidMethod(this->paintObj, this->setTextAlignId, this->Align_CENTER);
}

float Paint::ascent() {
    return env->CallFloatMethod(this->paintObj, this->ascentId);
}

float Paint::descent() {
    return env->CallFloatMethod(this->paintObj, this->descentId);
}

void Paint::setShadowLayer(float radius, float dx, float dy, int shadowColor) {
    return env->CallVoidMethod(this->paintObj, this->setShadowLayerId, radius, dx, dy, shadowColor);
}

void Paint::setTypeface(jobject typeface) {
    auto result = env->CallObjectMethod(this->paintObj, this->setTypefaceId, typeface);
    env->DeleteLocalRef(result);
}

void Paint::setAntiAlias(bool bUseAA) {
    return env->CallVoidMethod(this->paintObj, this->setAntiAliasId, bUseAA);
}

float Paint::measureText(const char *text) {
    auto str = env->NewStringUTF(text);
    auto f = env->CallFloatMethod(this->paintObj, this->measureTextId, str);
    env->DeleteLocalRef(str);
    return f;
}

float Paint::measureText(const wchar_t *text) {
    auto str = wcstojstr2(env, text);
    auto f = env->CallFloatMethod(this->paintObj, this->measureTextId, str);
    env->DeleteLocalRef(str);
    return f;
}