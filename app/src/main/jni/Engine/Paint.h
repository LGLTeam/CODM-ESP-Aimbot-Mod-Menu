//
// Created by aimar on 1/13/2020.
//

#ifndef PAINT_H
#define PAINT_H

#include <jni.h>

#include "Const.h"

#include "Canvas.h"
#include "Typeface.h"

class Paint {
public:
    JNIEnv *env;

    Rect *m_Rect;
    jobject paintObj;

    jmethodID setStyleId;
    jmethodID setTextSizeId;
    jmethodID setColorId;
    jmethodID getTextBoundsId;
    jmethodID setStrokeWidthId;
    jmethodID setTextAlignId;
    jmethodID ascentId;
    jmethodID descentId;
    jmethodID setShadowLayerId;
    jmethodID setTypefaceId;
    jmethodID setAntiAliasId;
    jmethodID measureTextId;

    jobject Style_FILL, Style_STROKE, Style_FILL_AND_STROKE;
    jobject Align_LEFT, Align_RIGHT, Align_CENTER;

    Paint(JNIEnv *env) {
        this->env = env;

        m_Rect = new Rect(env);

        jclass paintClass = env->FindClass("android/graphics/Paint");
        jmethodID init = env->GetMethodID(paintClass, "<init>", "()V");
        this->paintObj = env->NewGlobalRef(env->NewObject(paintClass, init));

        setStyleId = env->GetMethodID(paintClass, "setStyle", "(Landroid/graphics/Paint$Style;)V");
        setTextSizeId = env->GetMethodID(paintClass, "setTextSize", "(F)V");
        setColorId = env->GetMethodID(paintClass, "setColor", "(I)V");
        getTextBoundsId = env->GetMethodID(paintClass, "getTextBounds", "(Ljava/lang/String;IILandroid/graphics/Rect;)V");
        setStrokeWidthId = env->GetMethodID(paintClass, "setStrokeWidth", "(F)V");
        setTextAlignId = env->GetMethodID(paintClass, "setTextAlign", "(Landroid/graphics/Paint$Align;)V");
        ascentId = env->GetMethodID(paintClass, "ascent", "()F");
        descentId = env->GetMethodID(paintClass, "descent", "()F");
        setShadowLayerId = env->GetMethodID(paintClass, "setShadowLayer", "(FFFI)V");
        setTypefaceId = env->GetMethodID(paintClass, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
        setAntiAliasId = env->GetMethodID(paintClass, "setAntiAlias", "(Z)V");
        measureTextId = env->GetMethodID(paintClass, "measureText", "(Ljava/lang/String;)F");
        env->DeleteLocalRef(paintClass);

        jclass styleClass = env->FindClass("android/graphics/Paint$Style");
        jfieldID id = env->GetStaticFieldID(styleClass, "FILL", "Landroid/graphics/Paint$Style;");
        Style_FILL = env->NewGlobalRef(env->GetStaticObjectField(styleClass, id));
        id = env->GetStaticFieldID(styleClass, "STROKE", "Landroid/graphics/Paint$Style;");
        Style_STROKE = env->NewGlobalRef(env->GetStaticObjectField(styleClass, id));
        id = env->GetStaticFieldID(styleClass, "FILL_AND_STROKE", "Landroid/graphics/Paint$Style;");
        Style_FILL_AND_STROKE = env->NewGlobalRef(env->GetStaticObjectField(styleClass, id));
        env->DeleteLocalRef(styleClass);

        jclass alignClass = env->FindClass("android/graphics/Paint$Align");
        id = env->GetStaticFieldID(alignClass, "LEFT", "Landroid/graphics/Paint$Align;");
        Align_LEFT = env->NewGlobalRef(env->GetStaticObjectField(alignClass, id));
        id = env->GetStaticFieldID(alignClass, "RIGHT", "Landroid/graphics/Paint$Align;");
        Align_RIGHT = env->NewGlobalRef(env->GetStaticObjectField(alignClass, id));
        id = env->GetStaticFieldID(alignClass, "CENTER", "Landroid/graphics/Paint$Align;");
        Align_CENTER = env->NewGlobalRef(env->GetStaticObjectField(alignClass, id));
        env->DeleteLocalRef(alignClass);
    }

    void setStyle(Style style);
    void setTextSize(float size);
    void setColor(int color);
    Rect *getTextBounds(const char *text, int start, int end);
    Rect *getTextBounds(const wchar_t *text, int start, int end);
    void setStrokeWidth(float size = 1.0f);
    void setShadowLayer(float radius, float dx, float dy, int shadowColor);
    void setTextAlign(Align align);
    void setTypeface(jobject typeface);
    void setAntiAlias(bool bUseAA);
    float measureText(const char *text);
    float measureText(const wchar_t *text);
    float ascent();
    float descent();
};


#endif //ML_PAINT_H
