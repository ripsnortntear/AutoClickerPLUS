#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

#define IDC_START_BUTTON 101
#define IDC_STOP_BUTTON 102
#define IDC_SLIDER 103
#define IDC_LEFT_RADIO 104
#define IDC_RIGHT_RADIO 105
#define IDC_COUNTER_LABEL 106

HWND hSlider, hLeftRadio, hRightRadio, hCounterLabel;
bool clicking = false;
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
    switch (uMsg) {
    case WM_CREATE: {
        CreateWindow(L"BUTTON", L"Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 20, 80, 30, hwnd, (HMENU)IDC_START_BUTTON, NULL, NULL);

        CreateWindow(L"BUTTON", L"Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 20, 80, 30, hwnd, (HMENU)IDC_STOP_BUTTON, NULL, NULL);

        hLeftRadio = CreateWindow(L"BUTTON", L"Left Button", WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            20, 70, 100, 20, hwnd, (HMENU)IDC_LEFT_RADIO, NULL, NULL);
        hRightRadio = CreateWindow(L"BUTTON", L"Right Button", WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
            120, 70, 100, 20, hwnd, (HMENU)IDC_RIGHT_RADIO, NULL, NULL);
        SendMessage(hLeftRadio, BM_SETCHECK, BST_CHECKED, 0);

        hSlider = CreateWindow(TRACKBAR_CLASS, L"Slider", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            20, 110, 180, 30, hwnd, (HMENU)IDC_SLIDER, NULL, NULL);
        SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 1000));
        SendMessage(hSlider, TBM_SETPOS, TRUE, 100);

        hCounterLabel = CreateWindow(L"STATIC", L"Clicks: 0", WS_VISIBLE | WS_CHILD,
            20, 160, 180, 20, hwnd, (HMENU)IDC_COUNTER_LABEL, NULL, NULL);
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

    const wchar_t CLASS_NAME[] = L"AutoClickerWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Auto Clicker",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 240,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
