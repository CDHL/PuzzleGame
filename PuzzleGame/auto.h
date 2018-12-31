#pragma once

#include <cstdlib>
#include <queue>
#include <set>
#include <vector>

#include "game.h"

constexpr int STEP_BUFFER_SIZE = 1024;

extern int g_stepCount;
extern MoveInfo g_steps[STEP_BUFFER_SIZE];
extern bool g_threadRunning;

template<typename DATA_TYPE>
class Allocator
{
	static const int ALLOC_COUNT = 1024;
	std::vector<DATA_TYPE*> m_pool;
	int m_curElement;

public:

	Allocator() :m_curElement(ALLOC_COUNT) {}

	~Allocator()
	{
		for (DATA_TYPE *p : m_pool) delete[] p;
	}

	DATA_TYPE* getNew()
	{
		if (m_curElement == ALLOC_COUNT)
		{
			m_pool.push_back(new DATA_TYPE[ALLOC_COUNT]);
			m_curElement = 0;
		}
		return m_pool.back() + m_curElement++;
	}
};

template <class BoardType>
struct Status
{
	BoardType board;
	int f, g;
	Status *fa;

	void calcF();
};

template <>
void Status<Board<3> >::calcF();

template <>
void Status<Board<4> >::calcF();

template <>
void Status<Board<5> >::calcF();

template <class T>
struct Cmp
{
	bool operator() (T *const a, T *const b)
	{
		return a->f > b->f;
	}
};

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
			if (curEmpty == lastEmpty - 1) curMove = MOVE_RIGHT;
			else curMove = MOVE_DOWN;
		}
		else
		{
			if (curEmpty == lastEmpty + 1) curMove = MOVE_LEFT;
			else curMove = MOVE_UP;
		}
		g_steps[--curStep] = curMove;
		p = p->fa;
	}
	return true;
}

DWORD WINAPI AutoComplete(LPVOID lpParam);