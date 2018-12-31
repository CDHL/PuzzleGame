#include "stdafx.h"

#include "auto.h"

#include "game.h"
#include "window.h"

int g_stepCount;
MoveInfo g_steps[STEP_BUFFER_SIZE];
bool g_threadRunning = false;

template <>
void Status<Board<3> >::calcF()
{
	
}

template <>
void Status<Board<4> >::calcF()
{
	int tmp, trow, tcol;
	f = 0;
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			tmp = board.m_board[row * 4 + col];
			if (tmp == 4 * 4 - 1) continue;
			trow = tmp / 4;
			tcol = tmp % 4;
			f += abs(row - trow) + abs(col - tcol);
		}
	}
	f *= 7;
	f += g * 4;
}

template <>
void Status<Board<5> >::calcF()
{
	
}

DWORD WINAPI AutoComplete(LPVOID lpParam)
{
	g_threadRunning = true;
	InvalidateRect(g_hWnd, NULL, FALSE);

	bool res;
	if (g_boardSize == 4) res = GetSteps(g_board4);

	if (res)
	{
		for (int i = 0; i < g_stepCount; ++i)
		{
			if (g_boardSize == 4) g_board4.move(g_steps[i]);
			InvalidateRect(g_hWnd, NULL, FALSE);
			Sleep(400);
		}
	}

	g_threadRunning = false;
	InvalidateRect(g_hWnd, NULL, FALSE);
	return 0;
}