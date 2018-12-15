#include "draw.h"

#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <dwrite.h>

#include "game.h"

float DPIScale::scaleX = 1.0f;
float DPIScale::scaleY = 1.0f;

ID2D1Factory			*g_pFactory;
ID2D1HwndRenderTarget	*g_pRenderTarget;
ID2D1SolidColorBrush	*g_pBrush;

// 棋盘距边缘距离/min(窗口长, 窗口宽)
const float BORDER_GAP = 0.1f;
// 当前棋盘大小
int g_boardSize = 4;
// 棋盘边缘坐标（DIP）
float g_boardLeft, g_boardTop, g_boardRight, g_boardBottom;
// 单格边长（DIP）
float g_gridGap;
// 左键按下时的行列坐标
int g_lastPointX, g_lastPointY;

extern HWND hwnd;

// Recalculate drawing layout when the size of the window changes.
void CalculateLayout()
{
	if (g_pRenderTarget != NULL)
	{
		D2D1_SIZE_F size = g_pRenderTarget->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float halfSideLength = (0.5f - BORDER_GAP) * 2 * min(x, y);

		g_boardLeft = x - halfSideLength;
		g_boardTop = y - halfSideLength;
		g_boardRight = x + halfSideLength;
		g_boardBottom = y + halfSideLength;

		g_gridGap = halfSideLength * 2 / g_boardSize;
	}
}

HRESULT CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (g_pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		hr = g_pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hwnd,
				D2D1::SizeU(
					rc.right - rc.left,
					rc.bottom - rc.top
				)
			),
			&g_pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			hr = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &g_pBrush);

			if (SUCCEEDED(hr))
			{
				CalculateLayout();
			}
		}
	}
	return hr;
}

void DiscardGraphicsResources()
{
	SafeRelease(g_pRenderTarget);
	SafeRelease(g_pBrush);
}

void OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	
}

void OnLButtonUp(int pixelX, int pixelY, DWORD flags)
{
	
}

void OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);

		g_pRenderTarget->BeginDraw();

		g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		PaintBoard();
		// TODO: DrawText

		hr = g_pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}

		EndPaint(hwnd, &ps);
	}
}

void PaintBoard()
{
	g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
	D2D1_POINT_2F start, end;
	for (int i = 0; i <= g_boardSize; ++i)
	{
		start.x = g_boardLeft;
		start.y = g_boardTop + i * g_gridGap;
		end.x = g_boardRight;
		end.y = start.y;

		g_pRenderTarget->DrawLine(
			start,
			end,
			g_pBrush
		);

		start.x = g_boardLeft + i * g_gridGap;
		start.y = g_boardTop;
		end.x = start.x;
		end.y = g_boardBottom;

		g_pRenderTarget->DrawLine(
			start,
			end,
			g_pBrush
		);
	}
}

void Resize()
{
	if (g_pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		g_pRenderTarget->Resize(size);
		CalculateLayout();
		InvalidateRect(hwnd, NULL, FALSE);
	}
}