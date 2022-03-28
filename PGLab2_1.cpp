// PGLab2_1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PGLab2_1.h"
#include <map>
#include <list>
#include <iostream>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

const POINT field_size = { 25, 25 };
const POINT default_board_size = { 15, 15 };
const POINT min_board_size = { 7, 7 };
const POINT max_board_size = { 24, 30 };
const int default_num_of_bombs = 30;
const int min_num_of_bombs = 10;
const double default_bomb_size = 0.8;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterClassChild(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ChildProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    CustomSize(HWND, UINT, WPARAM, LPARAM);
void                GameOver(bool result, MainWindow* window);
void                Reset(MainWindow* main_window, POINT board_size, int num_of_bombs);
void                Setup(MainWindow* main_window, POINT board_size, int num_of_bombs);
void                MakeFieldWindows(MainWindow* mw);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    srand(time(NULL));

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PGLAB21, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterClassChild(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PGLAB21));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PGLAB21));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PGLAB21);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
ATOM MyRegisterClassChild(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ChildProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_HAND);
    wcex.hbrBackground = GetSysColorBrush(COLOR_GRAYTEXT);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"childClass";
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   MainWindow* mw = new MainWindow();

   Setup(mw, default_board_size, default_num_of_bombs);
   mw->window = CreateWindowExW(0L, szWindowClass, szTitle, 
       WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
       mw->centerPos.x, mw->centerPos.y, mw->size.x, mw->size.y,
       nullptr, nullptr, hInstance, mw);

   if (!mw->window) return FALSE;

   ShowWindow(mw->window, nCmdShow);
   UpdateWindow(mw->window);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MainWindow* mw;

    switch (message)
    {
    case WM_CREATE:
        {
            mw = (MainWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams);

            //Make field windows
            for (int i = 0; i < mw->game->board_size.x; i++) {
                for (int j = 0; j < mw->game->board_size.y; j++) {
                    FieldWindow* fw = new FieldWindow();
                    fw->field = &(mw->game->board[i][j]);
                    fw->main_window = mw;
                    mw->field_windows.push_back(fw);
                    fw->window = CreateWindowExW(0L, L"childClass", nullptr, 
                        (WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE),
                        i * field_size.x + 1, j * field_size.y + 30, field_size.x-1, field_size.y-1,
                        hWnd, nullptr, hInst, fw);
                    fw->field->Add_changed_func([fw]()
                        {
                            RECT rc;
                            GetClientRect(fw->window, &rc);
                            InvalidateRect(fw->window, &rc, true);
                        });
                }
            }

            for (auto it = mw->field_windows.begin(); it != mw->field_windows.end(); ++it) {
                ShowWindow((*it)->window, SW_SHOWNORMAL);
                UpdateWindow((*it)->window);
            }
        }
        break;
    case WM_TIMER:
        if (wParam == 1 && mw->game->running) {
            mw->time_from_start += 0.1;
            mw->game->changed();
        }
        break;
    case WM_MOVING:
        {
            RECT* rc = (RECT*)lParam;
            rc->left = mw->centerPos.x;
            rc->right = mw->centerPos.x + mw->size.x;
            rc->top = mw->centerPos.y;
            rc->bottom = mw->centerPos.y + mw->size.y;
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About, 0L);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_GAME_NEW:
                Reset(mw, mw->game->board_size, mw->game->num_of_bombs);
                break;
            case ID_GAME_CUSTOMSIZE:
                DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, CustomSize, (LPARAM)mw);
                break;
            case ID_HELP_DEBUG:
                {
                    HMENU hmenu = GetMenu(hWnd);

                    MENUITEMINFO menuItem = { 0 };
                    menuItem.cbSize = sizeof(MENUITEMINFO);
                    menuItem.fMask = MIIM_STATE;

                    GetMenuItemInfo(hmenu, ID_HELP_DEBUG, FALSE, &menuItem);

                    if (mw->debug){
                        mw->debug = false;
                        menuItem.fState = MFS_UNCHECKED;
                    }
                    else {
                        mw->debug = true;
                        menuItem.fState = MFS_CHECKED;
                    }
                    for (auto it = mw->field_windows.begin(); it != mw->field_windows.end(); ++it)
                        (*it)->field->changed();
                    SetMenuItemInfo(hmenu, ID_HELP_DEBUG, FALSE, &menuItem);
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            //Font
            HFONT font = CreateFont(-MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_BOLD,
                false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Arial "));
            HFONT oldFont = (HFONT)SelectObject(hdc, font);
            SetTextColor(hdc, 0x0000ff);
            
            //Timer
            RECT rc;
            TCHAR s[MAX_LOADSTRING];
            _stprintf_s(s, MAX_LOADSTRING, _T("%06.1f"), mw->time_from_start);
            GetClientRect(hWnd, &rc);
            rc.right /= 2;
            rc.bottom = rc.top + 30;
            DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            //Counter
            int disp_num = mw->game->num_of_bombs - mw->game->num_flagged;
            if(disp_num > 0) _stprintf_s(s, MAX_LOADSTRING, _T("%04d"), disp_num);
            else if(mw->game->correct_flagged != mw->game->num_of_bombs) 
                _stprintf_s(s, MAX_LOADSTRING, _T("000?"));
            else _stprintf_s(s, MAX_LOADSTRING, _T("0000"));
            GetClientRect(hWnd, &rc);
            rc.left += rc.right/2;
            rc.bottom = rc.top + 30;
            DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, oldFont);
            DeleteObject(font);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
LRESULT CALLBACK ChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::map<HWND, FieldWindow*>* fws = new std::map<HWND, FieldWindow*>();

    static bool l_button_down = false;
    static bool r_button_down = false;

    FieldWindow* fw;

    switch (message)
    {
    case WM_CREATE:
        fw = (FieldWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams);
        fws->insert(std::pair<HWND, FieldWindow*>(hWnd, fw));
        break;
    case WM_LBUTTONDOWN:
        if (!l_button_down){
            l_button_down = true;
            fw = fws->at(hWnd);
            if (!fw->main_window->started) {
                fw->main_window->started = true;
                SetTimer(fw->main_window->window, 1, 100, nullptr);
            }
            fw->field->Click();
        }
        break;
    case WM_LBUTTONUP:
        l_button_down = false;
        break;
    case WM_RBUTTONDOWN:
        if (!r_button_down) {
            r_button_down = true;
            fw = fws->at(hWnd);
            if (fw->main_window->started) {
                fw->field->ToggleFlag();
            }
        }
        break;
    case WM_RBUTTONUP:
        r_button_down = false;
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;

        fw = fws->at(hWnd);

        //Background
        if(fw->field->open){
            GetClientRect(hWnd, &rc);
            HBRUSH brush = CreateSolidBrush(fw->field->is_bomb ? 0x0000ff : 0xdddddd);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }

        //Bomb
        if ((fw->field->open || fw->main_window->show_bombs || fw->main_window->debug) 
            && fw->field->is_bomb) {
            GetClientRect(hWnd, &rc);
            HBRUSH brush = CreateSolidBrush(0x000000);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            Ellipse(hdc, rc.left + rc.right * default_bomb_size,
                rc.top + rc.bottom * default_bomb_size,
                rc.right * (1 - default_bomb_size) + 1,
                rc.bottom * (1 - default_bomb_size) + 1);
            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }
       
        //Number of neighbor bombs
        if ((fw->field->open || fw->main_window->debug)
            && !fw->field->is_bomb && fw->field->bomb_neighbours_count > 0){
            TCHAR s[MAX_LOADSTRING];
            HFONT font = CreateFont(-MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72),
                0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, 
                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Arial "));
            HFONT oldFont = (HFONT)SelectObject(hdc, font);

            SetBkMode(hdc, TRANSPARENT);
            int color;
            switch (fw->field->bomb_neighbours_count) {
            case 0:
                color = 0x000000;
                break;
            case 1:
                color = 0x304eba;
                break;
            case 2:
                color = 0x17cf70;
                break;
            case 3:
                color = 0xc9c42c;
                break;
            case 4:
                color = 0xed9118;
                break;
            case 5:
                color = 0xed1818;
                break;
            case 6:
                color = 0xdb18ed;
                break;
            case 7:
                color = 0xb818ed;
                break;
            default:
                color = 0x36204f;
                break;
            }
            SetTextColor(hdc, color);

            _stprintf_s(s, MAX_LOADSTRING, _T("%d"), fw->field->bomb_neighbours_count);
            GetClientRect(hWnd, &rc);
            DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, oldFont);
            DeleteObject(font);
        }

        //Draw flag
        if (fw->field->flagged) {
            HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
            BitBlt(hdc, 0, 0, 48, 48, memDC, 0, 0, SRCCOPY);
            RECT rc;
            GetClientRect(hWnd, &rc);
            StretchBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, memDC, 0, 0, 48, 48, SRCCOPY);
            SelectObject(memDC, oldBitmap);
            DeleteObject(bitmap);
            DeleteDC(memDC);
        }

        EndPaint(hWnd, &ps);
    }
        break;
    case WM_DESTROY:
        l_button_down = false;
        r_button_down = false;
        delete fws;
        fws = new std::map<HWND, FieldWindow*>();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK CustomSize(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MainWindow* mw;

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        mw = (MainWindow*)lParam;

        //Default values (from current game)
        SetDlgItemInt(hDlg, IDC_HEIGHTFIELD, mw->game->board_size.y, true);
        SetDlgItemInt(hDlg, IDC_WIDTHFIELD, mw->game->board_size.x, true);
        SetDlgItemInt(hDlg, IDC_MINEFIELD, mw->game->num_of_bombs, true);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        bool try_again = false;
        mw->game->running = false;
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (LOWORD(wParam) == IDOK) {
                POINT new_size;
                int new_mine_num;
                new_size.y = GetDlgItemInt(hDlg, IDC_HEIGHTFIELD, NULL, true);
                new_size.x = GetDlgItemInt(hDlg, IDC_WIDTHFIELD, NULL, true);
                new_mine_num = GetDlgItemInt(hDlg, IDC_MINEFIELD, NULL, true);
                
                //Validate input
                if (new_size.x < min_board_size.x
                    || new_size.y < min_board_size.y
                    || new_mine_num < min_num_of_bombs) {
                    MessageBox(hDlg,
                        L"Minimal board size is (7, 7), and minimal number of bombs is 10",
                        L"Incorrect input", MB_OK);
                    try_again = true;
                }
                else if (new_size.x > max_board_size.x
                    || new_size.y > max_board_size.y
                    || new_mine_num > new_size.x * new_size.y) {
                    MessageBox(hDlg,
                        L"Max board size is (24, 30), and number of bombs must be smaller than number of fields",
                        L"Incorrect input", MB_OK);
                    try_again = true;
                }
                else {
                    Reset(mw, new_size, new_mine_num);
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                }
            }
            if (!try_again) {
                EndDialog(hDlg, LOWORD(wParam));
                mw->game->running = true;
            }
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void GameOver(bool won, MainWindow* mw) {
    mw->show_bombs = true;
    for (auto it = mw->field_windows.begin(); it != mw->field_windows.end(); ++it)
        (*it)->field->changed();
    if (won) {
        MessageBox(mw->window, L"You won! Cool!", L"GG", MB_OK | MB_ICONEXCLAMATION);
    }
    else {
        int res = MessageBox(mw->window, L":(", L":/", MB_RETRYCANCEL | MB_ICONERROR);
        switch (res) {
        case 2:
            DestroyWindow(mw->window);
            break;
        case 4:
            Reset(mw, mw->game->board_size, mw->game->num_of_bombs);
            break;
        }
    }
}

void Reset(MainWindow* main_window, POINT board_size, int num_of_bombs) {
    main_window->game->running = false;

    for (auto it = main_window->field_windows.begin(); it != main_window->field_windows.end(); ++it) {
        DestroyWindow((*it)->window);
    }
    
    delete main_window->game;
    Setup(main_window, board_size, num_of_bombs);

    RECT rc;
    rc.left = main_window->centerPos.x;
    rc.right = main_window->centerPos.x + main_window->size.x;
    rc.top = main_window->centerPos.y;
    rc.bottom = main_window->centerPos.y + main_window->size.y;
    MoveWindow(main_window->window, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);

    MakeFieldWindows(main_window);

    InvalidateRect(main_window->window, NULL, true);
}
void Setup(MainWindow* mw, POINT board_size, int num_of_bombs) {
    mw->game = new Game(board_size, num_of_bombs, [mw](bool won) {GameOver(won, mw); });
    mw->game->AddUpdateCallback([mw]() {
        RECT rc;
        GetClientRect(mw->window, &rc);
        rc.bottom = rc.top + 30;
        InvalidateRect(mw->window, &rc, true);
        });
    mw->size = { mw->game->board_size.x * field_size.x + 1, mw->game->board_size.y * field_size.y + 30 };
    //mw->size = { mw->game->board_size.x * field_size.x + 16, mw->game->board_size.y * field_size.y + 88 };
    mw->centerPos = { (GetSystemMetrics(SM_CXSCREEN) - mw->size.x) / 2,
                    (GetSystemMetrics(SM_CYSCREEN) - mw->size.y) / 2 };

    RECT rc = { mw->centerPos.x, mw->centerPos.y, 
        mw->centerPos.x + mw->size.x, mw->centerPos.y + mw->size.y };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, true);
    mw->centerPos = { rc.left, rc.top };
    mw->size = { rc.right - rc.left, rc.bottom - rc.top };

    mw->started = false;
    mw->show_bombs = false;
    mw->time_from_start = 0;
    mw->field_windows = std::list<FieldWindow*>();
}
void MakeFieldWindows(MainWindow* mw) {
    for (int i = 0; i < mw->game->board_size.x; i++) {
        for (int j = 0; j < mw->game->board_size.y; j++) {
            FieldWindow* fw = new FieldWindow();
            fw->field = &(mw->game->board[i][j]);
            fw->main_window = mw;
            mw->field_windows.push_back(fw);
            fw->window = CreateWindowExW(0L, L"childClass", nullptr,
                (WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE),
                i * field_size.x + 1, j * field_size.y + 30, field_size.x - 1, field_size.y - 1,
                mw->window, nullptr, hInst, fw);
            fw->field->Add_changed_func([fw]()
                {
                    RECT rc;
                    GetClientRect(fw->window, &rc);
                    InvalidateRect(fw->window, &rc, true);
                });
        }
    }

    for (auto it = mw->field_windows.begin(); it != mw->field_windows.end(); ++it) {
        ShowWindow((*it)->window, SW_SHOWNORMAL);
        UpdateWindow((*it)->window);
    }
}
