#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

#define IDC_START_BUTTON     101
#define IDC_STOP_BUTTON      102
#define IDC_SLIDER           103
#define IDC_LEFT_RADIO       104
#define IDC_RIGHT_RADIO      105
#define IDC_COUNTER_LABEL    106
#define IDC_THEME_TOGGLE     107

HWND hSlider, hLeftRadio, hRightRadio, hCounterLabel, hThemeToggle;
bool clicking = false;
bool darkMode = false;
int clickCount = 0;
UINT_PTR timerId = 1;

void UpdateCounter() {
    std::wstring text = L"Clicks: " + std::to_wstring(clickCount);
    SetWindowText(hCounterLabel, text.c_str());
}

void DoClick(bool leftButton) {
    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = leftButton ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = leftButton ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

    SendInput(2, input, sizeof(INPUT));
    clickCount++;
    UpdateCounter();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HBRUSH hBrushLight = CreateSolidBrush(RGB(240, 240, 240));
    static HBRUSH hBrushDark = CreateSolidBrush(RGB(30, 30, 30));

    switch (uMsg) {
    case WM_CREATE: {
        CreateWindow(L"BUTTON", L"Start", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 20, 80, 30, hwnd, (HMENU)IDC_START_BUTTON, NULL, NULL);

        CreateWindow(L"BUTTON", L"Stop", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            120, 20, 80, 30, hwnd, (HMENU)IDC_STOP_BUTTON, NULL, NULL);

        hLeftRadio = CreateWindow(L"BUTTON", L"Left Click", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
            20, 70, 100, 20, hwnd, (HMENU)IDC_LEFT_RADIO, NULL, NULL);
        hRightRadio = CreateWindow(L"BUTTON", L"Right Click", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
            120, 70, 100, 20, hwnd, (HMENU)IDC_RIGHT_RADIO, NULL, NULL);
        SendMessage(hLeftRadio, BM_SETCHECK, BST_CHECKED, 0);

        hSlider = CreateWindow(TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS,
            20, 110, 180, 30, hwnd, (HMENU)IDC_SLIDER, NULL, NULL);
        SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 1000));
        SendMessage(hSlider, TBM_SETPOS, TRUE, 100);
        SendMessage(hSlider, TBM_SETTICFREQ, 100, 0);

        hCounterLabel = CreateWindow(L"STATIC", L"Clicks: 0", WS_VISIBLE | WS_CHILD,
            20, 150, 180, 20, hwnd, (HMENU)IDC_COUNTER_LABEL, NULL, NULL);

        hThemeToggle = CreateWindow(L"BUTTON", L"🌞", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 190, 60, 30, hwnd, (HMENU)IDC_THEME_TOGGLE, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_START_BUTTON:
            if (!clicking) {
                int interval = SendMessage(hSlider, TBM_GETPOS, 0, 0);
                SetTimer(hwnd, timerId, interval, NULL);
                clicking = true;
            }
            break;
        case IDC_STOP_BUTTON:
            if (clicking) {
                KillTimer(hwnd, timerId);
                clicking = false;
            }
            break;
        case IDC_THEME_TOGGLE:
            darkMode = SendMessage(hThemeToggle, BM_GETCHECK, 0, 0) == BST_CHECKED;
            SetWindowText(hThemeToggle, darkMode ? L"🌙" : L"🌞");
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)(darkMode ? hBrushDark : hBrushLight));
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        break;
    }
    case WM_TIMER: {
        bool leftButton = SendMessage(hLeftRadio, BM_GETCHECK, 0, 0) == BST_CHECKED;
        DoClick(leftButton);
        int interval = SendMessage(hSlider, TBM_GETPOS, 0, 0);
        KillTimer(hwnd, timerId);
        SetTimer(hwnd, timerId, interval, NULL);
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, darkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        return (INT_PTR)(darkMode ? hBrushDark : hBrushLight);
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, darkMode ? hBrushDark : hBrushLight);
        return TRUE;
    }
    case WM_DESTROY:
        KillTimer(hwnd, timerId);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES };
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"AutoClickerModern";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Auto Clicker",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 280,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
