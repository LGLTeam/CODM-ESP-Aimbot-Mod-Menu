#pragma once

#include <map>
#include <stdio.h>
#include <vector>
#include <string>

#ifndef ENGINE_CONST
#define ENGINE_CONST

#define ARGB(a, r, g, b) 0 | a << 24 | r << 16 | g << 8 | b

#define WHITE               ARGB(255, 255, 255, 255)
#define RED                 ARGB(255, 255, 000, 000)
#define GREEN               ARGB(255, 000, 128, 000)
#define LIME                ARGB(255, 000, 255, 000)
#define BLUE                ARGB(255, 000, 000, 255)
#define BLACK               ARGB(255, 000, 000, 000)
#define PURPLE              ARGB(255, 125, 000, 255)
#define GREY                ARGB(255, 128, 128, 128)
#define YELLOW              ARGB(255, 255, 255, 000)
#define ORANGE              ARGB(255, 255, 125, 000)
#define DARK_GREEN          ARGB(255, 000, 100, 000)
#define PINK                ARGB(255, 255, 192, 203)
#define BROWN               ARGB(255, 210, 105, 30)
#define CYAN                ARGB(255, 000, 255, 255)

enum Style {
    FILL = 0,
    STROKE = 1,
    FILL_AND_STROKE = 2
};

enum Align {
    LEFT = 0,
    RIGHT = 1,
    CENTER
};

struct Point {
    float x, y;
};

#endif