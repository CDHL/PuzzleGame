#include "stdafx.h"

#include "window.h"

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <windowsx.h>

#include "auto.h"
#include "draw.h"
#include "game.h"
#include "resource.h"

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

INT_PTR CALLBACK AboutDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;

	case WM_INITDIALOG:
		// 居中对话框（相对父窗口）
	{
		SendDlgItemMessage(hWnd, IDC_STATIC1, STM_SETICON, (WPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)), 0);
		HWND hWndOwner = GetParent(hWnd);
		RECT rcDlg, rcOwner;

		GetWindowRect(hWnd, &rcDlg);
		GetWindowRect(hWndOwner, &rcOwner);

		SetWindowPos(hWnd, HWND_TOP,
			(rcOwner.left + rcOwner.right - (rcDlg.right - rcDlg.left)) / 2,
			(rcOwner.top + rcOwner.bottom - (rcDlg.bottom - rcDlg.top)) / 2,
			0, 0, SWP_NOSIZE);
	}
	return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:          // Fall through to the next case.

		case NM_RETURN:
			// NOT supported MultiByte
			ShellExecute(NULL, L"open", ((PNMLINK)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

LRESULT CALLBACK BtnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		// 将键盘消息发送给主窗口，同时拦截空格键（按下按钮）
		SendMessage(g_hWnd, uMsg, wParam, lParam);
		return 0;

	case WM_KEYUP:
		SendMessage(g_hWnd, uMsg, wParam, lParam);
		return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void OnSelectImage()
{
	TCHAR fileName[260] = _T("");
	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.hwndOwner = g_hWnd;                     // 所有者窗口句柄
	ofn.lpstrFilter = _T("所有图片文件\0*.bmp;*.dib;*.jpg;*.jpeg;*.jpe;*.gif;*.tif;*.tiff;*.png;*.ico\0所有文件\0*.*\0");	// 过滤器
//	ofn.lpstrCustomFilter = NULL;               // 不保留用户定义的过滤器模式
	ofn.nFilterIndex = 1;                       // 默认选中的过滤器索引（第一对字符串的索引值为1，第二对字符串为2，依此类推。索引为零表示由lpstrCustomFilter指定的自定义筛选器	。）
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = 260;
//	ofn.lpstrFileTitle = NULL;                  // 所选文件的文件名和扩展名（没有路径信息）。该成员可以为NULL。
//	ofn.lpstrInitialDir = NULL;                 // 初始目录
//	ofn.lpstrTitle = NULL;                      // 要放在对话框标题栏中的字符串。如果此成员为NULL，则系统使用默认标题（即“另存为”或“打开”）。
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
	{
		SetImageFile(fileName);
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (lParam && HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case MBTN_DIFFICULTY:
				if (g_threadRunning) break;
				if (g_boardSize == 3) g_boardSize = 4;
				else if (g_boardSize == 4) g_boardSize = 5;
				else g_boardSize = 3;
				CalculateLayout();
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case MBTN_RANDOM:
				if (g_threadRunning) break;
				if (g_boardSize == 3) g_board3.random_shuffle();
				else if (g_boardSize == 4) g_board4.random_shuffle();
				else g_board5.random_shuffle();
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case MBTN_SOLVE:
				if (g_threadRunning) break;
				if (g_boardSize == 3) g_board3.clear();
				else if (g_boardSize == 4) g_board4.clear();
				else g_board5.clear();
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case MBTN_AUTO:
				if (g_threadRunning) break;
				g_threadRunning = true;
				InvalidateRect(g_hWnd, NULL, FALSE);
				CloseHandle(CreateThread(NULL, 0, AutoComplete, NULL, 0, NULL));
				break;

			case MBTN_STOP:
				g_threadRunning = false;
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case MBTN_IMAGE:
				OnSelectImage();
				break;
			}
		}
		return 0;

	case WM_CREATE:
		// 创建WIC工厂
		// CLSID_WICImagingFactory默认为CLSID_WICImagingFactory2
		// 使用CLSID_WICImagingFactory1以兼容Windows7及以下
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory))))
		{
			// 失败时尝试创建CLSID_WICImagingFactory1以兼容Windows7
			if (FAILED(CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory))))
			{
				return -1;
			}
		}
		// 创建Direct2D工厂
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pID2D1Factory)))
		{
			return -1;
		}
		DPIScale::Initialize();

		// 添加"关于"项到窗口菜单
		{
			HMENU hMenu = GetSystemMenu(hWnd, FALSE);
			AppendMenu(hMenu, MF_SEPARATOR, 0, 0);         // 添加水平分割线
			AppendMenu(hMenu, MF_STRING, MMENU_ABOUT, _T("关于 Puzzle Game(&A)..."));
		}
		// 创建按钮
		{
			HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

#ifndef _UNICODE
#error ComCtl32.dll version 6 is Unicode only. \
For more information, please visit https://docs.microsoft.com/en-us/windows/desktop/controls/subclassing-overview
#endif
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
			// 是否为自动重复功能产生
			if (lParam & 0x40000000) break;
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

		case 'W':

		case VK_UP:
			if (g_threadRunning) break;
			OnMove(MoveInfo::MOVE_UP);
			break;

		case 'S':

		case VK_DOWN:
			if (g_threadRunning) break;
			OnMove(MoveInfo::MOVE_DOWN);
			break;

		case 'A':

		case VK_LEFT:
			if (g_threadRunning) break;
			OnMove(MoveInfo::MOVE_LEFT);
			break;

		case 'D':

		case VK_RIGHT:
			if (g_threadRunning) break;
			OnMove(MoveInfo::MOVE_RIGHT);
			break;

		case VK_SPACE:
			if (!g_isPreview)
			{
				g_isPreview = true;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		}
		return 0;

	case WM_KEYUP:
		switch (wParam)
		{
		case VK_SPACE:
			if (g_isPreview)
			{
				g_isPreview = false;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		}
		return 0;

	case WM_LBUTTONDOWN:
		if (g_threadRunning) break;
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

	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		// 在窗口菜单中点击"关于"项
		case MMENU_ABOUT:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDialogProc);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
