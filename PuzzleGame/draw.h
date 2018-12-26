#pragma once

#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <wincodec.h>

#include "game.h"

extern ID2D1Factory            *g_pID2D1Factory;
extern ID2D1HwndRenderTarget   *g_pRenderTarget;
extern ID2D1SolidColorBrush    *g_pBrush;
extern IWICImagingFactory      *g_pIWICFactory;

extern int g_boardSize;

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
	static float scaleX;
	static float scaleY;

public:
	static void Initialize(ID2D1Factory *pFactory)
	{
		FLOAT dpiX, dpiY;

		pFactory->GetDesktopDpi(&dpiX, &dpiY);

		scaleX = dpiX / 96.0f;
		scaleY = dpiY / 96.0f;
	}

	// ����תΪDIPs���豸�޹����أ�
	template <typename T>
	static D2D1_POINT_2F PixelsToDips(T x, T y)
	{
		return D2D1::Point2F(static_cast<float>(x) / scaleX, static_cast<float>(y) / scaleY);
	}

	template <typename T>
	static float PixelsToDipsX(T x)
	{
		return static_cast<float>(x) / scaleX;
	}

	template <typename T>
	static float PixelsToDipsY(T y)
	{
		return static_cast<float>(y) / scaleY;
	}
};

// ���㲼��
void CalculateLayout();

// ����Direct2D��Դ
HRESULT CreateGraphicsResources();

// ����Direct2D��Դ
void DiscardGraphicsResources();

// �������
void OnLButtonDown(int pixelX, int pixelY, DWORD flags);

// ����ɿ�
void OnLButtonUp(int pixelX, int pixelY, DWORD flags);

// �ƶ�ͼ��
void OnMove(MoveInfo mov);

// ���ƴ���
void OnPaint();

// ��������
void PaintBoard();

// ���ư�ť
void PaintButton();

// �������ڴ�С
void Resize();

// ����ͼƬ�ļ�
HRESULT SetImageFile(PCTSTR fileName);