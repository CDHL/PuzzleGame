#include "stdafx.h"

#include "draw.h"

#include "game.h"
#include "window.h"

float DPIScale::scaleX = 1.0f;
float DPIScale::scaleY = 1.0f;

ID2D1Factory			*g_pID2D1Factory;
ID2D1HwndRenderTarget	*g_pRenderTarget;
ID2D1SolidColorBrush	*g_pBrush;
IWICImagingFactory      *g_pIWICFactory;

// 棋盘距边缘距离/min(窗口长, 窗口宽)
constexpr float BORDER_GAP = 0.1f;
// 绘制矩形长宽比
constexpr float RECT_SCALE = 16.0f / 9.0f;
// 按钮垂直间隔/按钮高度
constexpr float BUTTON_GAP = 1.0f / 4.0f;
// 当前棋盘大小
int g_boardSize = 4;
// 绘制矩形
RECT g_paintRect;
// 棋盘左上角坐标（DIP）
D2D1_POINT_2F g_boardLTPoint;
// 单格边长（DIP）
float g_gridGapX, g_gridGapY;
// 按钮长宽
int g_buttonWidth, g_buttonHeight;
// 左键按下时的行列坐标
int g_lastPointX, g_lastPointY;

void CalculateLayout()
{
	if (g_pRenderTarget != NULL)
	{
		// 获取客户区大小
		GetClientRect(g_hWnd, &g_paintRect);

		// 根据长宽比裁剪大小
		int width = static_cast<int>(g_paintRect.bottom * RECT_SCALE);
		int height;

		if (width < g_paintRect.right)
		{
			g_paintRect.left = (g_paintRect.right - width) / 2;
			g_paintRect.right = (g_paintRect.right + width) / 2;
			height = g_paintRect.bottom;
		}
		else
		{
			width = g_paintRect.right;
			height = static_cast<int>(g_paintRect.right / RECT_SCALE);
			g_paintRect.top = (g_paintRect.bottom - height) / 2;
			g_paintRect.bottom = (g_paintRect.bottom + height) / 2;
		}

		// 切掉边缘留白
		const int horizontalGap = static_cast<int>(width * BORDER_GAP);
		const int verticalGap = static_cast<int>(height * BORDER_GAP);

		g_paintRect.left += horizontalGap;
		g_paintRect.top += verticalGap;
		g_paintRect.right -= horizontalGap;
		g_paintRect.bottom -= verticalGap;
		width = g_paintRect.right - g_paintRect.left;
		height = g_paintRect.bottom - g_paintRect.top;

		// 计算棋盘位置
		g_boardLTPoint = DPIScale::PixelsToDips(g_paintRect.right - height, g_paintRect.top);
		g_gridGapX = DPIScale::PixelsToDipsX(height) / g_boardSize;
		g_gridGapY = DPIScale::PixelsToDipsY(height) / g_boardSize;

		// 计算按钮位置
		g_buttonWidth = width - height - horizontalGap;
		g_buttonHeight = static_cast<int>(height / (4 + (4 - 1) * BUTTON_GAP));
	}
}

HRESULT CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (g_pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(g_hWnd, &rc);

		hr = g_pID2D1Factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				g_hWnd,
				D2D1::SizeU(rc.right, rc.bottom)
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
		BeginPaint(g_hWnd, &ps);

		g_pRenderTarget->BeginDraw();

		g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		PaintBoard();

		hr = g_pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}

		PaintButton();

		EndPaint(g_hWnd, &ps);
	}
}

void PaintBoard()
{
	g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
	const float boardBottom = g_boardLTPoint.y + g_gridGapY * g_boardSize;
	const float boardRight = g_boardLTPoint.x + g_gridGapX * g_boardSize;
	float tmp;

	// 绘制横线
	for (int i = 0; i <= g_boardSize; ++i)
	{
		tmp = g_boardLTPoint.y + i * g_gridGapY;

		g_pRenderTarget->DrawLine(
			{ g_boardLTPoint.x, tmp },
			{ boardRight, tmp },
			g_pBrush
		);
	}
	// 绘制竖线
	for (int i = 0; i <= g_boardSize; ++i)
	{
		tmp = g_boardLTPoint.x + i * g_gridGapX;

		g_pRenderTarget->DrawLine(
			{ tmp, g_boardLTPoint.y },
			{ tmp, boardBottom },
			g_pBrush
		);
	}
}

void PaintButton()
{
	int cury = g_paintRect.top;
	int dy = static_cast<int>(g_buttonHeight * (1 + BUTTON_GAP));

	MoveWindow(g_hBtnDifficulty, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
	InvalidateRect(g_hBtnDifficulty, NULL, FALSE);

	cury += dy;
	ShowWindow(g_hBtnRandom, SW_SHOW);
	MoveWindow(g_hBtnRandom, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
	InvalidateRect(g_hBtnRandom, NULL, FALSE);

	cury += dy;

	cury += dy;
	MoveWindow(g_hBtnImage, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
	InvalidateRect(g_hBtnImage, NULL, FALSE);

	// UNDONE: 添加其余按钮
}

void Resize()
{
	if (g_pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(g_hWnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		g_pRenderTarget->Resize(size);
		CalculateLayout();
		InvalidateRect(g_hWnd, NULL, FALSE);
	}
}