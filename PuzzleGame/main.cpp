#include "stdafx.h"

#include <cstdlib>
#include <ctime>

#include "draw.h"
#include "resource.h"
#include "window.h"

int WINAPI _tWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ PTSTR pCmdLine, 
	_In_ int nCmdShow
)
{
	// 使用时间作为rand的种子
	srand(static_cast<unsigned int>(time(NULL)));

	// 初始化COM库
	if (FAILED(CoInitialize(NULL)))
	{
		return 0;
	}

	// Register the window class.

	const TCHAR CLASS_NAME[] = _T("PuzzleGame Window Class");

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.
	
	g_hWnd = CreateWindowEx(
		0,                      // Optional window styles
		CLASS_NAME,             // Window class
		_T("Puzzle Game"),      // Window text
		WS_OVERLAPPEDWINDOW,    // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (g_hWnd == NULL)
	{
		return 0;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	// Run the message loop.
	
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 取消初始化COM库
	CoUninitialize();

	return (int)msg.wParam;
}
