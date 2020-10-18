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

extern int cellsz;
void vitaeApp::DrawCell(UINT x, UINT y) {
    D2D1_POINT_2F point = D2D1::Point2F(0, 0);
    D2D1_POINT_2F point2 = D2D1::Point2F(0, 0);
    D2D1_RECT_F cell = D2D1::RectF();
    D2D1_ELLIPSE cellCircle = D2D1::Ellipse(point, 0, 0);
    if (cellFigure == SQUARE) {
        cell.top = static_cast<FLOAT>(y);
        cell.bottom = static_cast<FLOAT>(y) + cellsz;
        cell.left = static_cast<FLOAT>(x);
        cell.right = static_cast<FLOAT>(x) + cellsz;
        if (grid[x / cellsz][y / cellsz] == 1)
            m_pRenderTarget->FillRectangle(&cell, m_pCornflowerBlueBrush);
        else if (grid[x / cellsz][y / cellsz] == 0)
            m_pRenderTarget->FillRectangle(&cell, m_pWhiteBrush);
    }
    if (cellFigure == CIRCLE) {
        point.x = x + (cellsz / 2);
        point.y = y + (cellsz / 2);
        cellCircle.point = point;
        cellCircle.radiusX = cellsz / 2;
        cellCircle.radiusY = cellsz / 2;
        if (grid[x / cellsz][y / cellsz] == 1)
            m_pRenderTarget->FillEllipse(&cellCircle, m_pCornflowerBlueBrush);
        else if (grid[x / cellsz][y / cellsz] == 0)
            m_pRenderTarget->FillRectangle(&cell, m_pWhiteBrush);
    }
    if (cellFigure == LINE) {
        point.x = x;
        point.y = y;
        point2.x = x + cellsz;
        point2.y = y + cellsz;
        if (grid[x / cellsz][y / cellsz] == 1)
            m_pRenderTarget->DrawLine(point, point2, m_pCornflowerBlueBrush, 1.0F);
        else if (grid[x / cellsz][y / cellsz] == 0)
            m_pRenderTarget->DrawLine(point, point2, m_pWhiteBrush, 1.0F);
    }
}

void vitaeApp::RenderGrid() {
	int x, y;
	for (x = 0; x < 600; x += cellsz)
		for (y = 0; y < 600; y += cellsz) {
			DrawCell(x, y);
		}
}

void vitaeApp::DrawLine(UINT gx1, UINT gy1, UINT gx2, UINT gy2) {
	int x, y, dx, dy, a;
	float step2, fy, fx;
	dx = gx2 - gx1;
	if (dx < 0)
		dx = 0 - dx;
	dy = gy2 - gy1;
	if (dy < 0)
		dy = 0 - dy;
	if (dx > dy)
	{
		if (gx1 > gx2)
		{
			a = gx1;
			gx1 = gx2;
			gx2 = a;
			a = gy1;
			gy1 = gy2;
			gy2 = a;
		}
		step2 = (float)(gy2 - gy1) / (float)(gx2 - gx1);	//Slope (m)
		x = gx1;
		fy = gy1;
		do
		{
			grid[x][(int)fy] = 1;
			fy = fy + step2;
			x++;
		} while (x <= gx2);
	}
	else
	{
		if (gy1 > gy2)
		{
			a = gx1;
			gx1 = gx2;
			gx2 = a;
			a = gy1;
			gy1 = gy2;
			gy2 = a;
		}
		step2 = (float)(gx2 - gx1) / (float)(gy2 - gy1);	//Slope (m)
		y = gy1;
		fx = gx1;
		do {
			grid[(int)fx][y] = 1;
			fx = fx + step2;
			y++;
		} while (y <= gy2);
	}
	RenderGrid();
}
