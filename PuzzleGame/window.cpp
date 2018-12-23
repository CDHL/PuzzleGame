#include "stdafx.h"

#include "window.h"

#include <windowsx.h>

#include "draw.h"

HWND g_hWnd;

HWND g_hBtnDifficulty;
HWND g_hBtnRandom, g_hBtnSolve;
HWND g_hBtnAuto, g_hBtnStop;
HWND g_hBtnImage;

// 当前是否处于全屏状态
bool g_isFullScreen = false;
// 全屏前的窗口位置
RECT g_lastWindowRect;
// 全屏前的窗口样式
LONG_PTR g_lastWindowStyle;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case MBTN_DIFFICULTY:
			OutputDebugString(_T("BTN CLICK\n"));
			break;
		}
		return 0;

	case WM_CREATE:
		// 创建WIC工厂
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory))))
		{
			return -1;
		}
		// 创建Direct2D工厂
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pID2D1Factory)))
		{
			return -1;
		}
		DPIScale::Initialize(g_pID2D1Factory);

		// 创建按钮
		{
			HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

			g_hBtnDifficulty = CreateWindow(_T("BUTTON"), _T("更改难度"), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_DIFFICULTY, hInstance, NULL);
			SetWindowSubclass(g_hBtnDifficulty, BtnWindowProc, MBTN_DIFFICULTY, 0);

			g_hBtnRandom = CreateWindow(_T("BUTTON"), _T("随机打乱"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_RANDOM, hInstance, NULL);
			SetWindowSubclass(g_hBtnRandom, BtnWindowProc, MBTN_RANDOM, 0);

			g_hBtnSolve = CreateWindow(_T("BUTTON"), _T("解决"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_SOLVE, hInstance, NULL);
			SetWindowSubclass(g_hBtnSolve, BtnWindowProc, MBTN_SOLVE, 0);

			g_hBtnAuto = CreateWindow(_T("BUTTON"), _T("自动完成"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_AUTO, hInstance, NULL);
			SetWindowSubclass(g_hBtnAuto, BtnWindowProc, MBTN_AUTO, 0);

			g_hBtnStop = CreateWindow(_T("BUTTON"), _T("停止"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_STOP, hInstance, NULL);
			SetWindowSubclass(g_hBtnStop, BtnWindowProc, MBTN_STOP, 0);

			g_hBtnImage = CreateWindow(_T("BUTTON"), _T("自定义图片..."), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_IMAGE, hInstance, NULL);
			SetWindowSubclass(g_hBtnImage, BtnWindowProc, MBTN_IMAGE, 0);
		}

		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		SafeRelease(g_pID2D1Factory);
		SafeRelease(g_pIWICFactory);
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F11:
			// 全屏显示
			if (!g_isFullScreen)
			{
				GetWindowRect(hWnd, &g_lastWindowRect);
				g_lastWindowStyle = SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(hWnd, 0, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_NOZORDER);
			}
			// 取消全屏
			else
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, g_lastWindowStyle);
				SetWindowPos(hWnd, 0, g_lastWindowRect.left, g_lastWindowRect.top, 
					g_lastWindowRect.right - g_lastWindowRect.left, g_lastWindowRect.bottom - g_lastWindowRect.top, SWP_FRAMECHANGED | SWP_NOZORDER);
			}
			g_isFullScreen ^= 1;
			break;
		}
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK BtnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		// 将键盘消息发送给主窗口，同时拦截空格键（按下按钮）
		SendMessage(g_hWnd, uMsg, wParam, lParam);
		return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
