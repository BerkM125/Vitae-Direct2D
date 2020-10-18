#pragma once

#include "resource.h"
#define GRID_WIDTH 300
#define SQUARE 1
#define CIRCLE 2
#define LINE 3
using namespace std;

class vitaeApp {
    public:
        vitaeApp();
        ~vitaeApp();
        HRESULT Initialize();
        void RunMessageLoop();

    private:
        //Device resource functions:
        HRESULT CreateDeviceIndependentResources();
        HRESULT CreateDeviceResources();
        HRESULT OnRender();
        void DiscardDeviceResources();
        //Vitae default GOL functions
        void GenerateCycle();
        void RenderGrid();
        void RandFillGrid();
        void MainProcess();
        void DrawGridLines();
        void OnResize(
            UINT width,
            UINT height
        );
        //Cellular graphics functions:
        void DrawLine(int gx1, int gy1, int gx2, int gy2);
        void DrawCell(UINT x, UINT y);
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
        int boardsize = 600;
        int cellFigure = SQUARE;
        bool gridState = FALSE;
};