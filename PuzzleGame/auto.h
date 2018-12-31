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
	static const int ALLOC_COUNT = 8192;
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
Status<BoardType>* Astar(const BoardType &start, Allocator<Status<BoardType> > &alloc);

template <class BoardType>
bool GetSteps(const BoardType &start);

DWORD WINAPI AutoComplete(LPVOID lpParam);