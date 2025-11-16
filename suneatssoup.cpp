// SunEatsSoup.cpp
// Harmless, silly Windows GUI that draws a smiling sun, a bowl of soup,
// and an animated spoon that gently dips. No files, no network, no system changes.

#include <windows.h>
#include <tchar.h>
#include <math.h>

#define TIMER_ID 1
#define TIMER_MS 90

int g_spoonOffset = 0;
int g_spoonDir = 1; // 1 = forward, -1 = back

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, TIMER_MS, NULL);
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            g_spoonOffset += g_spoonDir * 4;
            if (g_spoonOffset > 40) g_spoonDir = -1;
            if (g_spoonOffset < 0) g_spoonDir = 1;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        // double buffer to reduce flicker
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // Clear background (sky)
        HBRUSH skyBrush = CreateSolidBrush(RGB(220, 245, 255));
        FillRect(memDC, &rc, skyBrush);
        DeleteObject(skyBrush);

        // Draw sun (top-left)
        int sunX = w / 6;
        int sunY = h / 6;
        int sunR = min(w, h) / 8;
        HBRUSH sunBrush = CreateSolidBrush(RGB(255, 215, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, sunBrush);
        Ellipse(memDC, sunX - sunR, sunY - sunR, sunX + sunR, sunY + sunR);
        SelectObject(memDC, oldBrush);
        DeleteObject(sunBrush);

        // Sun rays
        HPEN rayPen = CreatePen(PS_SOLID, 2, RGB(255, 195, 0));
        HPEN oldPen = (HPEN)SelectObject(memDC, rayPen);
        for (int i = 0; i < 8; ++i) {
            double ang = i * (3.14159 * 2 / 8);
            int x1 = (int)(sunX + cos(ang) * (sunR + 6));
            int y1 = (int)(sunY + sin(ang) * (sunR + 6));
            int x2 = (int)(sunX + cos(ang) * (sunR + 18));
            int y2 = (int)(sunY + sin(ang) * (sunR + 18));
            MoveToEx(memDC, x1, y1, NULL);
            LineTo(memDC, x2, y2);
        }
        SelectObject(memDC, oldPen);
        DeleteObject(rayPen);

        // Sun smile (simple arc)
        Arc(memDC, sunX - sunR/2, sunY - sunR/3, sunX + sunR/2, sunY + sunR/2, sunX + sunR/4, sunY + sunR/10, sunX - sunR/4, sunY + sunR/10);

        // Bowl (bottom center)
        int bowlW = w / 2;
        int bowlH = h / 6;
        int bowlX1 = (w - bowlW) / 2;
        int bowlX2 = bowlX1 + bowlW;
        int bowlY = h * 2 / 3;
        HBRUSH bowlBrush = CreateSolidBrush(RGB(200, 80, 120)); // cheerful bowl color
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, bowlBrush);
        // draw bowl body as a thick ellipse and then a rim
        Ellipse(memDC, bowlX1, bowlY, bowlX2, bowlY + bowlH);
        SelectObject(memDC, oldB);
        DeleteObject(bowlBrush);

        // Soup (fill inside bowl)
        HBRUSH soupBrush = CreateSolidBrush(RGB(255, 195, 100));
        RECT soupRect = { bowlX1 + 8, bowlY + 8, bowlX2 - 8, bowlY + bowlH - 6 };
        FillRect(memDC, &soupRect, soupBrush);
        DeleteObject(soupBrush);

        // Steam puffs above bowl (three arcs)
        HPEN steamPen = CreatePen(PS_SOLID, 2, RGB(180, 180, 180));
        SelectObject(memDC, steamPen);
        Arc(memDC, bowlX1 + bowlW/4 - 10, bowlY - 40, bowlX1 + bowlW/4 + 10, bowlY - 10, 0, 0, 0, 0);
        Arc(memDC, bowlX1 + bowlW/2 - 10, bowlY - 48, bowlX1 + bowlW/2 + 10, bowlY - 12, 0, 0, 0, 0);
        Arc(memDC, bowlX1 + 3*bowlW/4 - 10, bowlY - 44, bowlX1 + 3*bowlW/4 + 10, bowlY - 14, 0, 0, 0, 0);
        DeleteObject(steamPen);

        // Spoon (animated) - simple rounded rectangle and oval
        int spoonLen = bowlW / 2;
        int spoonBaseX = bowlX1 + bowlW/2 - spoonLen/2 + g_spoonOffset/2;
        int spoonBaseY = bowlY - 10 + g_spoonOffset/2;
        // handle
        HPEN spoonPen = CreatePen(PS_SOLID, 6, RGB(170, 170, 170));
        SelectObject(memDC, spoonPen);
        MoveToEx(memDC, spoonBaseX, spoonBaseY, NULL);
        LineTo(memDC, spoonBaseX + spoonLen, spoonBaseY - spoonLen/6);
        // bowl of spoon (oval)
        HBRUSH spoonBrush = CreateSolidBrush(RGB(200, 200, 200));
        int ovalX = spoonBaseX + spoonLen - 18;
        int ovalY = spoonBaseY - spoonLen/6 - 12;
        Ellipse(memDC, ovalX - 12, ovalY - 8, ovalX + 12, ovalY + 8);
        SelectObject(memDC, spoonBrush);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        DeleteObject(spoonBrush);
        DeleteObject(spoonPen);

        // Title text
        const TCHAR title[] = _T("SunEatsSoup");
        HFONT hFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        HFONT oldFont = (HFONT)SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(40, 40, 40));
        SetBkMode(memDC, TRANSPARENT);
        RECT titleRect = { 0, 8, w, 40 };
        DrawText(memDC, title, -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(memDC, oldFont);
        DeleteObject(hFont);

        // footer playful caption
        const char caption[] = "The sun is having soup! ☀️🍲";
#if defined(UNICODE) || defined(_UNICODE)
        // Convert to wide and draw
        WCHAR wcaption[128];
        MultiByteToWideChar(CP_UTF8, 0, caption, -1, wcaption, 128);
        HFONT smallFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                       DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        HFONT oldSmall = (HFONT)SelectObject(memDC, smallFont);
        RECT capRect = { 0, h - 30, w, h };
        DrawTextW(memDC, wcaption, -1, &capRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(memDC, oldSmall);
        DeleteObject(smallFont);
#else
        HFONT smallFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HFONT oldSmall = (HFONT)SelectObject(memDC, smallFont);
        RECT capRect = { 0, h - 30, w, h };
        DrawTextA(memDC, caption, -1, &capRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(memDC, oldSmall);
        DeleteObject(smallFont);
#endif

        // blit double buffer to screen
        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        // cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const TCHAR CLASS_NAME[] = _T("SunEatsSoupClass");

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        _T("SunEatsSoup 🥣☀️"),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 360,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
