// Visual Vitae.cpp : Defines the entry point for the application.
//
#pragma warning(suppress : 4996)
#pragma comment(lib, "d2d1.lib")
    // Windows Header Files:
#include <windows.h>
#include <windowsx.h>
// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <process.h>
#include <thread>
#include "framework.h"
#include "Visual Vitae.h"

template<class Interface>
inline void SafeRelease(
    Interface** ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif
#define MAX_LOADSTRING 100

// Global Variables:
vitaeApp::vitaeApp() :
    m_hwnd(NULL),
    m_pDirect2dFactory(NULL),
    m_pRenderTarget(NULL),
    m_pLightSlateGrayBrush(NULL),
    m_pCornflowerBlueBrush(NULL)
{
}

vitaeApp::~vitaeApp()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLightSlateGrayBrush);
    SafeRelease(&m_pCornflowerBlueBrush);
}

void vitaeApp::RunMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
//External variables:
int cellsz;
//Global windows handles/objects:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND mainHandle;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HRESULT vitaeApp::Initialize()
{
    HRESULT hr;
    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();
    wsprintf(szTitle, L"Vitae");
    wsprintf(szWindowClass, L"VitaeClass");

    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = vitaeApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = szWindowClass;

        RegisterClassEx(&wcex);


        // Because the CreateWindow function takes its size in pixels,
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use
        // to create its own windows.
        //#pragma warning(disable : 4996)
        m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
        // Create the window
        m_hwnd = CreateWindow(
            szWindowClass,
            szTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(600.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(600.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
        );
        mainHandle = m_hwnd;
        CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), m_hwnd, ControlsDialog);
        cellsz = static_cast<UINT>(ceil(600.f * dpiX / 96.f)) / GRID_WIDTH;
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT vitaeApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;
    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    return hr;
}

HRESULT vitaeApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
        );
        // Create a Direct2D render target.
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_MULTI_THREADED,
            &m_pDirect2dFactory
        );
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );

        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        //Initialize the grid(s)
        for(int i = 0; i < GRID_WIDTH; i++)
            for (int i2 = 0; i2 < GRID_WIDTH; i2++) {
                grid[i][i2] = 0;
                gridRep[i][i2] = 0;
            }
        generationLimit = 512;
        if (SUCCEEDED(hr))
        {
            // Create a gray brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                &m_pLightSlateGrayBrush
            );
        }
        if (SUCCEEDED(hr))
        {
            // Create a blue brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                &m_pCornflowerBlueBrush
            );
        }
        if (SUCCEEDED(hr))
        {
            // Create a white brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &m_pWhiteBrush
            );
        }
        if (SUCCEEDED(hr))
        {
            // Create the variable brush, which will can be changed in color by the user.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pVariableBrush
            );
        }
    }

    return hr;
}

void vitaeApp::GenerateCycle() {
    int a, b, l, right, r, c, x, y, n;

    for (r = 0; r < GRID_WIDTH; r++)
    {
        if (r > 0)
            a = r - 1;
        else
            a = GRID_WIDTH - 1;
        if (r < GRID_WIDTH - 1)
            b = r + 1;
        else
            b = 0;
        for (c = 0; c < GRID_WIDTH; c++)
        {
            if (c > 0)
                l = c - 1;
            else
                l = GRID_WIDTH - 1;

            if (c < GRID_WIDTH - 1)
                right = c + 1;
            else
                right = 0;
            n = 0;
            if (grid[a][c] == 1)
                n++;
            if (grid[a][right] == 1)
                n++;
            if (grid[a][l] == 1)
                n++;
            if (grid[r][right] == 1)
                n++;
            if (grid[r][l] == 1)
                n++;
            if (grid[b][c] == 1)
                n++;
            if (grid[b][right] == 1)
                n++;
            if (grid[b][l] == 1)
                n++;
            if (n == 3 || (grid[r][c] == 1 && n == 2))
                gridRep[r][c] = 1;
            else
                gridRep[r][c] = 0;
        }
    }

    for (x = 0; x < GRID_WIDTH; x++) {
        for (y = 0; y < GRID_WIDTH; y++) {
            grid[x][y] = gridRep[x][y];
        }
    }

    gen += 1;
}

void vitaeApp::RandFillGrid() {
    for (int x = 0; x < 600; x += cellsz) {
        for (int y = 0; y < 600; y += cellsz) {
            int state = rand() % 2;
            grid[x/cellsz][y/cellsz] = state;
        }
    }
}

void vitaeApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLightSlateGrayBrush);
    SafeRelease(&m_pCornflowerBlueBrush);
}

void vitaeApp::MainProcess() {
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
    // Draw a grid background.
    int width = static_cast<int>(rtSize.width);
    int height = static_cast<int>(rtSize.height);
    // Render the initial board
    GenerateCycle();
    RenderGrid();
    //Render grid lines.
    if (gridState == TRUE) {
        for (int x = 0; x < boardsize; x += cellsz)
        {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                D2D1::Point2F(static_cast<FLOAT>(x), boardsize),
                m_pLightSlateGrayBrush,
                0.5f
            );
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                D2D1::Point2F(boardsize, static_cast<FLOAT>(x)),
                m_pLightSlateGrayBrush,
                0.5f
            );
        }
    }
    else if (gridState == FALSE) {
        for (int x = 0; x < boardsize; x += cellsz)
        {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                D2D1::Point2F(static_cast<FLOAT>(x), boardsize),
                m_pWhiteBrush,
                0.5f
            );
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                D2D1::Point2F(boardsize, static_cast<FLOAT>(x)),
                m_pWhiteBrush,
                0.5f
            );
        }
    }
    
}

HRESULT vitaeApp::OnRender()
{
    HRESULT hr = S_OK;
    wchar_t glString[16];
    wsprintfW(glString, L"\0");
    hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        wsprintfW(glString, L"%d", generationLimit);
        switch (MessageBox(NULL, glString, L"Generation Count", MB_OK)) {
        case IDOK:
            break;
        }
        for (int r = 0; r < generationLimit; r++) {
            m_pRenderTarget->BeginDraw();
            MainProcess();
            hr = m_pRenderTarget->EndDraw();
        }
    }
    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }
    return hr;
}

LRESULT CALLBACK vitaeApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    static int x1 = 0;
    static int y1 = 0;
    static int x2 = 0;
    static int y2 = 0;
    static int state = 0;
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        vitaeApp* pVitaeApp = (vitaeApp*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pVitaeApp)
        );

        result = 1;
    }
    else
    {
        vitaeApp* pVitaeApp = reinterpret_cast<vitaeApp*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));
        bool wasHandled = false;

        if (pVitaeApp)
        {
            switch (message)
            {
                case WM_SIZE:
                    {
                        UINT width = LOWORD(lParam);
                        UINT height = HIWORD(lParam);
                        pVitaeApp->OnResize(width, height);
                    }
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_DISPLAYCHANGE:
                    InvalidateRect(hwnd, NULL, FALSE);
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_PAINT:
                    {
                        pVitaeApp->m_pRenderTarget->BeginDraw();
                        pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                        pVitaeApp->RenderGrid();
                        if (pVitaeApp->gridState == TRUE) {
                            for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                            {
                                pVitaeApp->m_pRenderTarget->DrawLine(
                                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                                    D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                                    pVitaeApp->m_pLightSlateGrayBrush,
                                    0.5f
                                );
                                pVitaeApp->m_pRenderTarget->DrawLine(
                                    D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                                    D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                                    pVitaeApp->m_pLightSlateGrayBrush,
                                    0.5f
                                );
                            }
                        }
                        else if (pVitaeApp->gridState == FALSE) {
                            for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                            {
                                pVitaeApp->m_pRenderTarget->DrawLine(
                                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                                    D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                                    pVitaeApp->m_pWhiteBrush,
                                    0.5f
                                );
                                pVitaeApp->m_pRenderTarget->DrawLine(
                                    D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                                    D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                                    pVitaeApp->m_pWhiteBrush,
                                    0.5f
                                );
                            }
                        }
                        pVitaeApp->m_pRenderTarget->EndDraw();
                        ValidateRect(hwnd, NULL);
                    }
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_DESTROY:
                    {
                        PostQuitMessage(0);
                    }
                    result = 1;
                    wasHandled = true;
                    break;
                case WM_MOUSEMOVE:
                    {
                        if (wParam & MK_LBUTTON) {
                            int xPos = GET_X_LPARAM(lParam);
                            int yPos = GET_Y_LPARAM(lParam);
                            if (state == 0) {
                                x1 = xPos / cellsz;
                                y1 = yPos / cellsz;
                                pVitaeApp->grid[x1][y1] = 1;
                                pVitaeApp->m_pRenderTarget->BeginDraw();
                                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                                pVitaeApp->RenderGrid();
                                if (pVitaeApp->gridState == TRUE) {
                                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                                    {
                                        pVitaeApp->m_pRenderTarget->DrawLine(
                                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                                            pVitaeApp->m_pLightSlateGrayBrush,
                                            0.5f
                                        );
                                        pVitaeApp->m_pRenderTarget->DrawLine(
                                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                                            pVitaeApp->m_pLightSlateGrayBrush,
                                            0.5f
                                        );
                                    }
                                }
                                else if (pVitaeApp->gridState == FALSE) {
                                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                                    {
                                        pVitaeApp->m_pRenderTarget->DrawLine(
                                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                                            pVitaeApp->m_pWhiteBrush,
                                            0.5f
                                        );
                                        pVitaeApp->m_pRenderTarget->DrawLine(
                                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                                            pVitaeApp->m_pWhiteBrush,
                                            0.5f
                                        );
                                    }
                                }
                                pVitaeApp->m_pRenderTarget->EndDraw();
                                state = 1;
                            }
                            else {
                                x2 = xPos / cellsz;
                                y2 = yPos / cellsz;
                                pVitaeApp->m_pRenderTarget->BeginDraw();
                                pVitaeApp->DrawLine(x1, y1, x2, y2);
                                pVitaeApp->m_pRenderTarget->EndDraw();
                                x1 = x2;
                                y1 = y2;
                            }
                        }
                    }
                default:
                    break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

void vitaeApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
)
{
    // Use HeapSetInformation to specify that the process should
    // terminate if the heap manager detects an error in any heap used
    // by the process.
    // The return value is ignored, because we want to continue running in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL))) {
        {
            vitaeApp app;
            if (SUCCEEDED(app.Initialize())) {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }
    return 0;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISUALVITAE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VISUALVITAE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
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
            // TODO: Add any drawing code that uses hdc here...
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

INT_PTR CALLBACK vitaeApp::ControlsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    vitaeApp* pVitaeApp = reinterpret_cast<vitaeApp*>(static_cast<LONG_PTR>(
        ::GetWindowLongPtrW(
            mainHandle,
            GWLP_USERDATA
        )));
    pVitaeApp->CreateDeviceResources();
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
        switch (LOWORD(wParam)) {
            case IDC_BUTTON1:
                pVitaeApp->OnRender();
                break;
            case IDC_BUTTON2:
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->RandFillGrid();
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_BUTTON3:
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->GenerateCycle();
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_BUTTON4:
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                for (int i = 0; i < GRID_WIDTH; i++) {
                    for (int i2 = 0; i2 < GRID_WIDTH; i2++) {
                        pVitaeApp->grid[i][i2] = 0;
                    }
                }
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_BUTTON5:
            {
                char gl[16];
                int glLong;
                sprintf_s(gl, "\0");
                GetDlgItemTextA(hDlg, IDC_EDIT1, gl, 4);
                sscanf_s(gl, "%d", &glLong);
                pVitaeApp->generationLimit = glLong;
            }
            case IDC_RADIO2:
                pVitaeApp->cellFigure = CIRCLE;
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_RADIO3:
                pVitaeApp->cellFigure = SQUARE;
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_RADIO1:
                pVitaeApp->cellFigure = LINE;
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
            case IDC_CHECK1:
                pVitaeApp->gridState = IsDlgButtonChecked(hDlg, IDC_CHECK1);
                pVitaeApp->m_pRenderTarget->BeginDraw();
                pVitaeApp->m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
                pVitaeApp->RenderGrid();
                if (pVitaeApp->gridState == TRUE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pLightSlateGrayBrush,
                            0.5f
                        );
                    }
                }
                else if (pVitaeApp->gridState == FALSE) {
                    for (int x = 0; x < pVitaeApp->boardsize; x += cellsz)
                    {
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                            D2D1::Point2F(static_cast<FLOAT>(x), pVitaeApp->boardsize),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                        pVitaeApp->m_pRenderTarget->DrawLine(
                            D2D1::Point2F(0.0f, static_cast<FLOAT>(x)),
                            D2D1::Point2F(pVitaeApp->boardsize, static_cast<FLOAT>(x)),
                            pVitaeApp->m_pWhiteBrush,
                            0.5f
                        );
                    }
                }
                pVitaeApp->m_pRenderTarget->EndDraw();
                break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

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
