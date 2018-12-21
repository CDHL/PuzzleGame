#include "stdafx.h"

#include "window.h"

#include <windowsx.h>

#include "draw.h"

HWND g_hWnd;

HWND g_hBtnDifficulty;
HWND g_hBtnRandom, g_hBtnSolve;
HWND g_hBtnAuto, g_hBtnStop;
HWND g_hBtnImage;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_DIFFICULTY:
			OutputDebugString(_T("BTN CLICK\n"));
			break;
		}
		return 0;

	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pFactory)))
		{
			return -1;
		}
		DPIScale::Initialize(g_pFactory);

		// 创建按钮
		g_hBtnDifficulty = CreateWindow(_T("BUTTON"), _T("更改难度"), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)BTN_DIFFICULTY, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnDifficulty, BtnWindowProc, BTN_DIFFICULTY, 0);

		g_hBtnRandom = CreateWindow(_T("BUTTON"), _T("随机打乱"), WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)BTN_RANDOM, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnRandom, BtnWindowProc, BTN_RANDOM, 0);

		g_hBtnSolve = CreateWindow(_T("BUTTON"), _T("解决"), WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)BTN_SOLVE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnSolve, BtnWindowProc, BTN_SOLVE, 0);

		g_hBtnAuto = CreateWindow(_T("BUTTON"), _T("自动完成"), WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)BTN_AUTO, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnAuto, BtnWindowProc, BTN_AUTO, 0);

		g_hBtnStop = CreateWindow(_T("BUTTON"), _T("停止"), WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)BTN_STOP, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnStop, BtnWindowProc, BTN_STOP, 0);

		g_hBtnImage = CreateWindow(_T("BUTTON"), _T("自定义图片"), WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)BTN_IMAGE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
		SetWindowSubclass(g_hBtnImage, BtnWindowProc, BTN_IMAGE, 0);

		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		SafeRelease(g_pFactory);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK BtnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		SendMessage(g_hWnd, uMsg, wParam, lParam);
		return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
