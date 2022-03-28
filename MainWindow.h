#pragma once
#include "framework.h"
#include <list>
#include "field.h"
#include "game.h"
#include "FieldWindow.h"

class MainWindow {
public:
    HWND window = nullptr;
    POINT size = { 0, 0 };
    POINT centerPos = { 0, 0 };

    std::list<FieldWindow*> field_windows = std::list<FieldWindow*>();

    Game* game = nullptr;
    float time_from_start = 0;
    bool started = false;

    bool show_bombs = false;
    bool debug = false;
};