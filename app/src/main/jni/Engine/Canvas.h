//
// Created by aimar on 1/13/2020.
//

#ifndef CANVAS_H
#define CANVAS_H

#include <jni.h>

#include "Const.h"

#include "Typeface.h"
#include "Rect.h"
#include "Path.h"
#include "Paint.h"

class Canvas
{
private:
    jobject m_CanvasObj, m_CurrentTypeface, clearObj;
    jmethodID drawTextId;
    jmethodID drawRectId;
    jmethodID drawLineId;
    jmethodID drawCircleId;
    jmethodID drawColorId;
    jmethodID drawPathId;
    jmethodID rotateId;
    jmethodID restoreId;

    int m_Width, m_Height; float m_Density;
    bool low;

    Rect *m_Rect;
    Typeface *m_Typeface;
    Path* m_Path;

    class Paint *m_TextPaint;
    class Paint *m_LinePaint;
    class Paint *m_FillPaint;
    class Paint *m_Paint;
public:
    JNIEnv *env;
    Canvas(JNIEnv *env, int width, int height, float density);

    void UpdateCanvas(jobject canvas);
    void LowMode(bool b);

    float scaleSize(float size);
    int scaleSize(int size);

    void drawText(const char *text, float X, float Y, float size = 10, Align align = Align::LEFT, int textColor = 0xFFFFFFFF, int outlineColor = 0x0);
    void drawText(const wchar_t *text, float X, float Y, float size = 10, Align align = Align::LEFT, int textColor = 0xFFFFFFFF, int outlineColor = 0x0);
    void drawText(std::vector<std::string> s, float X, float Y, float size = 10, Align align = Align::LEFT, std::vector<int> textColor=std::vector<int>(), std::vector<int> outlineColor=std::vector<int>());
    void drawText(std::vector<std::wstring> s, float X, float Y, float size = 10, Align align = Align::LEFT, std::vector<int> textColor=std::vector<int>(), std::vector<int> outlineColor=std::vector<int>());
    void drawBox(float X, float Y, float width, float height, int color = 0xFFFFFFFF);
    void drawBorder(float X, float Y, float width, float height, float thicc, int color = 0xFFFFFFFF);
    void drawBorderRect(float left, float top, float right, float bottom, float thicc, int color = 0xFFFFFFFF);
    void drawLine(float startX, float startY, float stopX, float stopY, float thicc, int color = 0xFFFFFFFF);
    void drawCircle(float x, float y, float radius, float thicc, bool fill = false, int color = 0xFFFFFFFF);
    void drawFilledTriangle(Point a, Point b, Point c, int color = 0xFFFFFFFF);
    void rotate(float degrees);
    void restore();

    Rect *getTextBounds(const char *text, int start, int end);
    Rect *getTextBounds(const wchar_t *text, int start, int end);

    float measureText(const char *text);
    float measureText(const wchar_t *text);
};


#endif //ML_CANVAS_H
