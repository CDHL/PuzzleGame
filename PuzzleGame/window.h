#pragma once

#define BTN_DIFFICULTY	    0x0001
#define BTN_RANDOM          0x0002
#define BTN_SOLVE           0x0003
#define BTN_AUTO            0x0004
#define BTN_STOP            0x0005
#define BTN_IMAGE           0x0006

extern HWND g_hWnd;

extern HWND g_hBtnDifficulty;
extern HWND g_hBtnRandom, g_hBtnSolve;
extern HWND g_hBtnAuto, g_hBtnStop;
extern HWND g_hBtnImage;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK BtnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
