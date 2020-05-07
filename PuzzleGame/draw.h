#pragma once

#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <wincodec.h>

#include "game.h"

extern ID2D1Factory            *g_pID2D1Factory;
extern ID2D1HwndRenderTarget   *g_pRenderTarget;
extern ID2D1SolidColorBrush    *g_pBrush;
extern IWICImagingFactory      *g_pIWICFactory;

extern bool g_isPreview;

template <class T> inline void SafeRelease(T *&ppT)
{
	if (ppT)
	{
		ppT->Release();
		ppT = NULL;
	}
}

class DPIScale
{
	static float scale;

public:
	static void Initialize()
	{
		scale = GetDpiForSystem() / 96.0f;
	}

	// 像素转为DIPs（设备无关像素）
	template <typename T>
	static D2D1_POINT_2F PixelsToDips(T x, T y)
	{
		return D2D1::Point2F(static_cast<float>(x) / scale, static_cast<float>(y) / scale);
	}

	template <typename T>
	static float PixelsToDips(T x)
	{
		return static_cast<float>(x) / scale;
	}
};

// 计算布局
void CalculateLayout();

// 创建Direct2D资源
HRESULT CreateGraphicsResources();

HRESULT CreateMyBitmapFromWicBitmap(IWICBitmapSource * pIWICBitmapSource);

// 放弃Direct2D资源
void DiscardGraphicsResources();

HRESULT LoadResourceBitmap();

// 左键按下
void OnLButtonDown(int pixelX, int pixelY, DWORD flags);

// 左键松开
void OnLButtonUp(int pixelX, int pixelY, DWORD flags);

// 移动图块
void OnMove(MoveInfo mov);

// 绘制窗口
void OnPaint();

// 绘制棋盘
void PaintBoard();

// 绘制按钮
void PaintButton();

// 调整窗口大小
void Resize();

// 更改图片文件
HRESULT SetImageFile(PCTSTR fileName);