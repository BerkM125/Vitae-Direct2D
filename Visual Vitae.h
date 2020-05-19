#pragma once

#include "resource.h"
#define GRID_WIDTH 100
class vitaeApp {
public:
    vitaeApp();
    ~vitaeApp();
    HRESULT Initialize();
    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    HRESULT OnRender();
    void DiscardDeviceResources();
    void GenerateCycle();
    void RenderGrid();
    void RandFillGrid();
    void MainProcess();
    void OnResize(
        UINT width,
        UINT height
    );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    static INT_PTR CALLBACK ControlsDialog(
        HWND hDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
    ID2D1SolidColorBrush* m_pWhiteBrush;
    ID2D1SolidColorBrush* m_pVariableBrush;
    int grid[GRID_WIDTH][GRID_WIDTH];
    int gridRep[GRID_WIDTH][GRID_WIDTH];
    int gen;
    int generationLimit;
};