#pragma once
#include "framework.h"
#include <list>
#include "game.h"
#include "field.h"
#include "MainWindow.h"

class MainWindow;
class FieldWindow {
public:
    HWND window;
    MainWindow* main_window;

    Field* field;
};
