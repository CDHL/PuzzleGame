#include "stdafx.h"

#include "auto.h"

#include "draw.h"
#include "game.h"
#include "window.h"

int g_stepCount;
MoveInfo g_steps[STEP_BUFFER_SIZE];
bool g_threadRunning = false;

template <>
void Status<Board<3> >::calcF()
{
	// 3 * 3 的棋盘 A* 退化为 BFS
	f = g;
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
	f *= 5;
	f += g * 4;
}

template <>
void Status<Board<5> >::calcF()
{
	int tmp, trow, tcol;
	f = 0;
	for (int row = 0; row < 5; ++row)
	{
		for (int col = 0; col < 5; ++col)
		{
			tmp = board.m_board[row * 5 + col];
			if (tmp == 5 * 5 - 1) continue;
			trow = tmp / 5;
			tcol = tmp % 5;
			f += abs(row - trow) + abs(col - tcol);
		}
	}
	f *= 9;
	f += g * 4;
}

template <class BoardType>
Status<BoardType>* Astar(const BoardType &start, Allocator<Status<BoardType> > &alloc)
{
	using Status = Status<BoardType>;
	using ULL = unsigned long long;

	std::priority_queue<Status*, std::vector<Status*>, Cmp<Status> > que;
	std::set<ULL> hashSet;
	std::pair<std::set<ULL>::iterator, bool> insertResult;

	Status *cur;
	Status *p = alloc.getNew();
	p->board = start;
	p->g = 0;
	p->fa = NULL;
	p->calcF();
	hashSet.insert(start.hash());
	que.push(p);

	while (!que.empty())
	{
		cur = que.top();
		if (cur->board.isFinished()) return cur;
		que.pop();

		for (int i = 0; i < 4; ++i)
		{
			if (cur->board.move(static_cast<MoveInfo>(i)))
			{
				insertResult = hashSet.insert(cur->board.hash());
				if (insertResult.second)
				{
					p = alloc.getNew();
					p->board = cur->board;
					p->g = cur->g + 1;
					p->calcF();
					p->fa = cur;
					que.push(p);
				}
				cur->board.move(static_cast<MoveInfo>(i ^ 1));
			}
		}
	}
	return NULL;
}

template <class BoardType>
bool GetSteps(const BoardType &start)
{
	using Status = Status<BoardType>;

	Allocator<Status> alloc;

	Status *res = Astar(start, alloc), *p;

	if (!res) return false;

	g_stepCount = 0;
	// 统计步数
	p = res;
	while (p->fa)
	{
		++g_stepCount;
		p = p->fa;
	}

	// 记录每步操作
	int curStep = g_stepCount;
	int lastEmpty, curEmpty;
	MoveInfo curMove;
	p = res;
	while (p->fa)
	{
		lastEmpty = p->fa->board.getEmpty();
		curEmpty = p->board.getEmpty();
		if (curEmpty < lastEmpty)
		{
			if (curEmpty == lastEmpty - 1) curMove = MoveInfo::MOVE_RIGHT;
			else curMove = MoveInfo::MOVE_DOWN;
		}
		else
		{
			if (curEmpty == lastEmpty + 1) curMove = MoveInfo::MOVE_LEFT;
			else curMove = MoveInfo::MOVE_UP;
		}
		g_steps[--curStep] = curMove;
		p = p->fa;
	}
	return true;
}

DWORD WINAPI AutoComplete(LPVOID lpParam)
{
	bool res;
	if (g_boardSize == 3) res = GetSteps(g_board3);
	else if (g_boardSize == 4) res = GetSteps(g_board4);
	else res = GetSteps(g_board5);

	if (res)
	{
		for (int i = 0; i < g_stepCount; ++i)
		{
			if (!g_threadRunning) return 0;
			OnMove(g_steps[i]);
			if (i != g_stepCount - 1) Sleep(300);
		}
	}

	g_threadRunning = false;
	InvalidateRect(g_hWnd, NULL, FALSE);
	return 0;
}