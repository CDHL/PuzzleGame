#include "stdafx.h"

#include "window.h"

#include <windowsx.h>

#include "draw.h"
#include "resource.h"

HWND g_hWnd;

HWND g_hBtnDifficulty;
HWND g_hBtnRandom, g_hBtnSolve;
HWND g_hBtnAuto, g_hBtnStop;
HWND g_hBtnImage;

// ��ǰ�Ƿ���ȫ��״̬
bool g_isFullScreen = false;
// ȫ��ǰ�Ĵ���λ��
RECT g_lastWindowRect;
// ȫ��ǰ�Ĵ�����ʽ
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
		// ���жԻ�����Ը����ڣ�
	{
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
		// ��������Ϣ���͸������ڣ�ͬʱ���ؿո�������°�ť��
		SendMessage(g_hWnd, uMsg, wParam, lParam);
		return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void OnSelectImage()
{
	TCHAR fileName[260] = _T("");
	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.hwndOwner = g_hWnd;                     // �����ߴ��ھ��
	ofn.lpstrFilter = _T("All files\0*.*\0PNG File\0*.png\0");	// ������
//	ofn.lpstrCustomFilter = NULL;               // �������û�����Ĺ�����ģʽ
	ofn.nFilterIndex = 2;                       // Ĭ��ѡ�еĹ�������������һ���ַ���������ֵΪ1���ڶ����ַ���Ϊ2���������ơ�����Ϊ���ʾ��lpstrCustomFilterָ�����Զ���ɸѡ��	����
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = 260;
//	ofn.lpstrFileTitle = NULL;                  // ��ѡ�ļ����ļ�������չ����û��·����Ϣ�����ó�Ա����ΪNULL��
//	ofn.lpstrInitialDir = NULL;                 // ��ʼĿ¼
//	ofn.lpstrTitle = NULL;                      // Ҫ���ڶԻ���������е��ַ���������˳�ԱΪNULL����ϵͳʹ��Ĭ�ϱ��⣨�������Ϊ���򡰴򿪡�����
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
				OutputDebugString(_T("BTN CLICK\n"));
				break;

			case MBTN_IMAGE:
				OnSelectImage();
				break;
			}
		}
		return 0;

	case WM_CREATE:
		// ����WIC����
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory))))
		{
			return -1;
		}
		// ����Direct2D����
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pID2D1Factory)))
		{
			return -1;
		}
		DPIScale::Initialize(g_pID2D1Factory);

		// ���"����"����ڲ˵�
		{
			HMENU hMenu = GetSystemMenu(hWnd, FALSE);
			AppendMenu(hMenu, MF_SEPARATOR, 0, 0);         // ���ˮƽ�ָ���
			AppendMenu(hMenu, MF_STRING, MMENU_ABOUT, _T("���� PuzzleGame(&A)..."));
		}
		// ������ť
		{
			HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

#ifndef _UNICODE
#error ComCtl32.dll version 6 is Unicode only. \
For more information, please visit https://docs.microsoft.com/en-us/windows/desktop/controls/subclassing-overview
#endif
			g_hBtnDifficulty = CreateWindow(_T("BUTTON"), _T("�����Ѷ�"), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_DIFFICULTY, hInstance, NULL);
			SetWindowSubclass(g_hBtnDifficulty, BtnWindowProc, MBTN_DIFFICULTY, 0);

			g_hBtnRandom = CreateWindow(_T("BUTTON"), _T("�������"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_RANDOM, hInstance, NULL);
			SetWindowSubclass(g_hBtnRandom, BtnWindowProc, MBTN_RANDOM, 0);

			g_hBtnSolve = CreateWindow(_T("BUTTON"), _T("���"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_SOLVE, hInstance, NULL);
			SetWindowSubclass(g_hBtnSolve, BtnWindowProc, MBTN_SOLVE, 0);

			g_hBtnAuto = CreateWindow(_T("BUTTON"), _T("�Զ����"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_AUTO, hInstance, NULL);
			SetWindowSubclass(g_hBtnAuto, BtnWindowProc, MBTN_AUTO, 0);

			g_hBtnStop = CreateWindow(_T("BUTTON"), _T("ֹͣ"), WS_CHILD | BS_PUSHBUTTON,
				0, 0, 0, 0, hWnd, (HMENU)MBTN_STOP, hInstance, NULL);
			SetWindowSubclass(g_hBtnStop, BtnWindowProc, MBTN_STOP, 0);

			g_hBtnImage = CreateWindow(_T("BUTTON"), _T("�Զ���ͼƬ..."), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
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
			// �Ƿ�Ϊ�Զ��ظ����ܲ���
			if (lParam & 0x40000000) break;
			// ȫ����ʾ
			if (!g_isFullScreen)
			{
				GetWindowRect(hWnd, &g_lastWindowRect);
				g_lastWindowStyle = SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(hWnd, 0, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_NOZORDER);
			}
			// ȡ��ȫ��
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

	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		// �ڴ��ڲ˵��е��"����"��
		case MMENU_ABOUT:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDialogProc);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
