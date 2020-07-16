#include <Windows.h>
#include <cmath>
#include <thread>

constexpr LPCWSTR className = L"Mandelbrot set";
int iterationsNum = 256 * 6;
long double step = 0.001;
long double offsetX = 0.0, offsetY = 0.0;
int wndSizeX, wndSizeY;
int colorOffset = 0;
int* value = nullptr;
bool g_shouldBeRecalculated = true;

int isBlowUp(long double creal, long double cim = 0) {
    //is cardioid 
    {
        long double psq = (-creal - 0.25) * (-creal - 0.25) + cim * cim;
        long double theta = atan2(cim, -creal - 0.25);
        long double pc = 0.5 - 0.5 * cos(theta);
        if (psq <= pc * pc) {
            return -1;
        }
    }

    long double real = 0, im = 0;
    for (int i = 0; i < iterationsNum; i++) {
        long double tmp = real;
        real = (real - im) * (real + im);
        im = 2 * tmp * im;
        real -= creal;
        im += cim;
        if (real * real + im * im > 4) {
            return i;
        }
    }

    return -1;
}

void gradient(long long value, byte& inBlue, byte& inGreen, byte& inRed) {
    if (value == -1) {
        inBlue = 0x00;
        inGreen = 0x00;
        inRed = 0x00;
        return;
    }
    value %= 256 * 6;

    char red;
    char grn;
    char blu;
    long long xxxx = value % 256;
    switch ((long long)(value) / 256) {
    default:// [[fallthrough]]
    case 6:// [[fallthrough]]
    case 0:
        red = 0x00; grn = 0x00; blu = xxxx;
        break;
    case 1:
        red = 0x00; grn = xxxx; blu = 0xff;
        break;
    case 2:
        red = 0x00; grn = 0xff; blu = 0xff - xxxx;
        break;
    case 3:
        red = xxxx; grn = 0xff; blu = 0x00;
        break;
    case 4:
        red = 0xff; grn = 0xff - xxxx; blu = 0x00;
        break;
    case 5:
        red = 0xff - xxxx; grn = 0x00; blu = 0x00;
        break;
    }

    /*if (ratio > 2.0 / 3.0) {
        blu = 0xff;//
        grn = 0xff;
        red = 3 * (ratio - 2.0 / 3.0) * 255;
    }
    else if (ratio > 1.0 / 3.0) {
        blu = 0xff;
        grn = 3 * (ratio - 1.0 / 3.0) * 255;
        red = 0x00;
    }
    else {
        blu = ratio * 3 * 255;
        grn = 0x00;
        red = 0x00;
    }*/

    inRed = red;
    inGreen = grn;
    inBlue = blu;
}

void parsePixels(int y1, int y2, int* pixelMap) {
    int iterator = y1 * wndSizeX;
    for (int iy = y1; iy < y2; iy++) {
        for (int ix = 0; ix < wndSizeX; ix++) {
            int z;
            pixelMap[iterator] = isBlowUp(step * (ix - (wndSizeX / 2)) + offsetX,
                step * (iy - (wndSizeY / 2)) + offsetY);

            iterator++;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    static int mzX = 0, mzY = 0;

    switch (msg) {
    case WM_MOUSEMOVE:
        mzX = LOWORD(lParam);
        mzY = HIWORD(lParam);
        break;
    case WM_LBUTTONDOWN:
        offsetX -= step * (mzX - wndSizeX / 2);
        offsetY += step * (mzY - wndSizeY / 2);
        g_shouldBeRecalculated = true;
        InvalidateRect(hWnd, 0, false);
        break;

    case WM_SIZE:
        if (wParam != SIZE_MAXIMIZED && wParam != SIZE_MINIMIZED) {
            break;
        }
    case WM_EXITSIZEMOVE:
    {
        delete[]value;

        RECT r;
        GetClientRect(hWnd, &r);
        wndSizeX = r.right - r.left;
        wndSizeX += 4 - wndSizeX % 4;
        wndSizeY += 4 - wndSizeY % 4;
        wndSizeY = r.bottom - r.top;
        value = new int[wndSizeX * wndSizeY];
        g_shouldBeRecalculated = true;
        InvalidateRect(hWnd, 0, false);
    }
    break;

    case WM_CREATE:
    {
        RECT r;
        GetClientRect(hWnd, &r);
        wndSizeX = r.right - r.left;
        wndSizeY = r.bottom - r.top;
        wndSizeX += 4 - wndSizeX % 4;
        wndSizeY += 4 - wndSizeY % 4;
        value = new int[wndSizeX * wndSizeY];
        g_shouldBeRecalculated = true;
    }
    break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_UP:
            offsetY -= step * 256;
            g_shouldBeRecalculated = true;
            break;

        case VK_DOWN:
            offsetY += step * 256;
            g_shouldBeRecalculated = true;
            break;

        case VK_LEFT:
            offsetX += step * 256;
            g_shouldBeRecalculated = true;
            break;

        case VK_RIGHT:
            offsetX -= step * 256;
            g_shouldBeRecalculated = true;
            break;

        case VK_OEM_MINUS:
            step *= 2;
            g_shouldBeRecalculated = true;
            break;

        case VK_OEM_PLUS:
            step /= 2;
            g_shouldBeRecalculated = true;
            break;

        case VK_NUMPAD2:
            iterationsNum <<= 1;
            if (iterationsNum == 0)
                iterationsNum = 2;
            g_shouldBeRecalculated = true;
            break;
        case VK_NUMPAD1:
            iterationsNum >>= 1;
            g_shouldBeRecalculated = true;
            break;

        case VK_NUMPAD0:
            iterationsNum = 256 * 6;
            step = 0.001;
            offsetX = 0;
            offsetY = 0;
            g_shouldBeRecalculated = true;
            break;

        }
        InvalidateRect(hWnd, 0, 0);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        {
            constexpr int threadNum = 4;
            HDC hDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, wndSizeX, wndSizeY);
            SelectObject(hDC, hbm);

            byte* pixel = new byte[wndSizeX * wndSizeY * 3];

            if (g_shouldBeRecalculated) {
                std::thread th1(parsePixels, 0, wndSizeY / 4, value);
                std::thread th2(parsePixels, wndSizeY / 4, wndSizeY / 2, value);
                std::thread th3(parsePixels, wndSizeY / 2, 3 * wndSizeY / 4, value);
                std::thread th4(parsePixels, 3 * wndSizeY / 4, wndSizeY, value);

                th1.join();
                th2.join();
                th3.join();
                th4.join();
                g_shouldBeRecalculated = false;
            }

            for (int i = 0; i < wndSizeX * wndSizeY; i++) {
                gradient(value[wndSizeX * wndSizeY - 1 - i], pixel[i * 3], pixel[i * 3 + 1], pixel[i * 3 + 2]);
            }

            BITMAPINFOHEADER bheader;
            ZeroMemory(&bheader, sizeof(bheader));
            bheader.biSize = sizeof(bheader);
            bheader.biWidth = wndSizeX;
            bheader.biHeight = wndSizeY;
            bheader.biPlanes = 1;
            bheader.biBitCount = 24;
            bheader.biCompression = BI_RGB;

            BITMAPINFO binf;
            ZeroMemory(&binf, sizeof(binf));
            binf.bmiHeader = bheader;

            int b = SetDIBits(hDC, hbm, 0, wndSizeY, pixel, &binf, DIB_PAL_COLORS);
            int a = GetLastError();

            delete[]pixel;
            BitBlt(hdc, 0, 0, wndSizeX, wndSizeY, hDC, 0, 0, SRCCOPY);
        }

        {
            SetPixel(hdc, wndSizeX / 2, wndSizeY / 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2 + 1, wndSizeY / 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2 - 1, wndSizeY / 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2, wndSizeY / 2 + 1, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2, wndSizeY / 2 - 1, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2 + 2, wndSizeY / 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2 - 2, wndSizeY / 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2, wndSizeY / 2 + 2, RGB(255, 0, 0));
            SetPixel(hdc, wndSizeX / 2, wndSizeY / 2 - 2, RGB(255, 0, 0));
        }

        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        delete[]value;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(wc);
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(hInst, IDC_ARROW);
    wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
    wc.hInstance = hInst;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = className;
    wc.lpszMenuName = NULL;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(className, L"Mandelbrot Set", WS_OVERLAPPEDWINDOW, 30, 30, 1600, 939, NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    BOOL ret = 1;

    while (ret != 0) {
        ret = GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (ret > 0) return 0;
    else return -1;
}