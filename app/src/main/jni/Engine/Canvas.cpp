//
// Created by aimar on 1/13/2020.
//

#include "Canvas.h"
#include "Rect.h"
#include "Path.h"
#include <wchar.h>
#include <string.h>

jstring wcstojstr(JNIEnv *env, const wchar_t *input) {
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

Canvas::Canvas(JNIEnv *env, int width, int height, float density) {
    this->env = env;

    this->low = false;

    this->m_Width = width;
    this->m_Height = height;
    this->m_Density = density;

    jclass canvasClass = env->FindClass("android/graphics/Canvas");
    drawTextId = env->GetMethodID(canvasClass, "drawText", "(Ljava/lang/String;FFLandroid/graphics/Paint;)V");
    drawRectId = env->GetMethodID(canvasClass, "drawRect", "(FFFFLandroid/graphics/Paint;)V");
    drawLineId = env->GetMethodID(canvasClass, "drawLine", "(FFFFLandroid/graphics/Paint;)V");
    drawCircleId = env->GetMethodID(canvasClass, "drawCircle", "(FFFLandroid/graphics/Paint;)V");
    drawColorId = env->GetMethodID(canvasClass, "drawColor", "(ILandroid/graphics/PorterDuff$Mode;)V");
    drawPathId = env->GetMethodID(canvasClass, "drawPath", "(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
    rotateId = env->GetMethodID(canvasClass, "rotate", "(F)V");
    restoreId = env->GetMethodID(canvasClass, "restore", "()V");
    env->DeleteLocalRef(canvasClass);

    jclass cls = env->FindClass("android/graphics/PorterDuff$Mode");
    auto id = env->GetStaticFieldID(cls, "CLEAR", "Landroid/graphics/PorterDuff$Mode;");
    clearObj = env->NewGlobalRef(env->GetStaticObjectField(cls, id));
    env->DeleteLocalRef(cls);

    m_Rect = new Rect(this->env);
    m_Typeface = new Typeface(this->env);
    m_CurrentTypeface = env->NewGlobalRef(m_Typeface->create("Arial Rounded MT", (int) FontStyle::BOLD));
    m_Path = new Path(this->env);

    m_TextPaint = new Paint(this->env);
    m_TextPaint->setTypeface(this->m_CurrentTypeface);
    m_TextPaint->setStyle(Style::FILL);
    m_TextPaint->setAntiAlias(true);

    m_LinePaint = new Paint(this->env);
    m_LinePaint->setStyle(Style::STROKE);
    m_LinePaint->setAntiAlias(true);

    m_FillPaint = new Paint(this->env);
    m_FillPaint->setStyle(Style::FILL);
    m_FillPaint->setAntiAlias(true);

    m_Paint = new Paint(this->env);
    m_Paint->setStyle(Style::FILL_AND_STROKE);
    m_Paint->setAntiAlias(true);
}

float Canvas::scaleSize(float size) {
    return size * m_Density;
}

int Canvas::scaleSize(int size) {
    return (int) ((float) size * m_Density);
}

void Canvas::UpdateCanvas(jobject canvas) {
    this->m_CanvasObj = canvas;
}

void Canvas::LowMode(bool b) {
    this->low = b;

    m_TextPaint->setAntiAlias(b);
    m_LinePaint->setAntiAlias(b);
    m_FillPaint->setAntiAlias(b);
    m_Paint->setAntiAlias(b);
}

void Canvas::drawText(const char *text, float X, float Y, float size, Align align, int textColor, int outlineColor) {
    Paint *paint = this->m_TextPaint;

    paint->setTextSize(this->scaleSize(size));
    paint->setTextAlign(align);

    jstring str = env->NewStringUTF(text);

    if (!low) {
        paint->setColor(outlineColor);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, str, X - 1, Y - 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, str, X + 1, Y + 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, str, X - 1, Y + 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, str, X + 1, Y - 1, paint->paintObj);
    }

    paint->setColor(textColor);
    env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, str, X, Y, paint->paintObj);

    env->DeleteLocalRef(str);
}

void Canvas::drawText(const wchar_t *text, float X, float Y, float size, Align align, int textColor, int outlineColor) {
    Paint *paint = this->m_TextPaint;

    paint->setTextSize(this->scaleSize(size));
    paint->setTextAlign(align);

    jstring convertedStr = wcstojstr(env, text);

    if (!low) {
        paint->setColor(outlineColor);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, convertedStr, X - 1, Y - 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, convertedStr, X + 1, Y + 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, convertedStr, X - 1, Y + 1, paint->paintObj);
        env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, convertedStr, X + 1, Y - 1, paint->paintObj);
    }

    paint->setColor(textColor);
    env->CallVoidMethod(this->m_CanvasObj, this->drawTextId, convertedStr, X, Y, paint->paintObj);

    env->DeleteLocalRef(convertedStr);
}

void Canvas::drawText(std::vector<std::string> s, float X, float Y, float size, Align align, std::vector<int> textColor, std::vector<int> outlineColor) {
    std::vector<float> centerPos;
    if (align == CENTER) {

    }
    int lastTextColor = 0, lastOutlineColor = 0;
    for (int i = 0; i < s.size(); i++) {
        int c = textColor.size() >= i ? textColor[i] : lastTextColor;
        lastTextColor = c;
        int oc = outlineColor.size() >= i ? outlineColor[i] : lastOutlineColor;
        lastOutlineColor = oc;
        drawText(s[i].c_str(), X, Y, size, align, c, oc);

        if (align == LEFT)
            X += m_TextPaint->measureText(s[i].c_str());
        else if (align == CENTER)
            X -= m_TextPaint->measureText(s[i].c_str());
        else if (align == RIGHT)
            X -= m_TextPaint->measureText(s[i].c_str());
    }
}

void Canvas::drawText(std::vector<std::wstring> s, float X, float Y, float size, Align align, std::vector<int> textColor, std::vector<int> outlineColor) {
    int lastTextColor = 0, lastOutlineColor = 0;
    for (int i = 0; i < s.size(); i++) {
        int c = textColor.size() >= i ? textColor[i] : lastTextColor;
        lastTextColor = c;
        int oc = outlineColor.size() >= i ? outlineColor[i] : lastOutlineColor;
        lastOutlineColor = oc;
        drawText(s[i].c_str(), X, Y, size, align, c, oc);

        if (align == LEFT)
            X += m_TextPaint->measureText(s[i].c_str());
        else if (align == CENTER)
            X += m_TextPaint->measureText(s[i].c_str()) + (m_TextPaint->measureText(s[i].c_str()) / 2);
        else if (align == RIGHT)
            X -= m_TextPaint->measureText(s[i].c_str());
    }
}

void Canvas::drawBox(float X, float Y, float width, float height, int color) {
    Paint *paint = this->m_FillPaint;

    paint->setColor(color);

    env->CallVoidMethod(this->m_CanvasObj, this->drawRectId, X, Y, X + width, Y + height, paint->paintObj);
}

void Canvas::drawBorder(float X, float Y, float width, float height, float thicc, int color) {
    Paint *paint = this->m_LinePaint;

    paint->setColor(color);
    paint->setStrokeWidth(thicc);

    env->CallVoidMethod(this->m_CanvasObj, this->drawRectId, X, Y, X + width, Y + height, paint->paintObj);
}

void Canvas::drawBorderRect(float left, float top, float right, float bottom, float thicc, int color) {
    Paint *paint = this->m_LinePaint;

    paint->setColor(color);
    paint->setStrokeWidth(thicc);

    env->CallVoidMethod(this->m_CanvasObj, this->drawRectId, left, top, right, bottom, paint->paintObj);
}

void Canvas::drawLine(float startX, float startY, float stopX, float stopY, float thicc, int color) {
    Paint *paint = this->m_LinePaint;

    paint->setColor(color);
    paint->setStrokeWidth(thicc);

    env->CallVoidMethod(this->m_CanvasObj, this->drawLineId, startX, startY, stopX, stopY, paint->paintObj);
}

void Canvas::drawCircle(float x, float y, float radius, float thicc, bool fill, int color) {
    Paint *paint = this->m_Paint;

    if (!fill)
        paint->setStyle(Style::STROKE);
    else
        paint->setStyle(Style::FILL);

    paint->setColor(color);
    paint->setStrokeWidth(thicc);

    env->CallVoidMethod(this->m_CanvasObj, this->drawCircleId, x, y, radius, paint->paintObj);
}

void Canvas::drawFilledTriangle(Point a, Point b, Point c, int color) {
    Paint *paint = this->m_FillPaint;

    paint->setColor(color);

    Path *path = this->m_Path;
    path->reset();
    path->moveTo(a.x, a.y);
    path->lineTo(b.x, b.y);
    path->lineTo(c.x, c.y);
    path->lineTo(a.x, a.y);
    path->close();

    env->CallVoidMethod(this->m_CanvasObj, this->drawPathId, path->pathObj, paint->paintObj);
}

void Canvas::rotate(float degrees) {
    return env->CallVoidMethod(this->m_CanvasObj, this->rotateId, degrees);
}

void Canvas::restore() {
    return env->CallVoidMethod(this->m_CanvasObj, this->restoreId);
}

Rect *Canvas::getTextBounds(const char *text, int start, int end) {
    return m_TextPaint->getTextBounds(text, start, end);
}

Rect *Canvas::getTextBounds(const wchar_t *text, int start, int end) {
    return m_TextPaint->getTextBounds(text, start, end);
}

float Canvas::measureText(const char *text) {
    return m_TextPaint->measureText(text);
}

float Canvas::measureText(const wchar_t *text) {
    return m_TextPaint->measureText(text);
}
