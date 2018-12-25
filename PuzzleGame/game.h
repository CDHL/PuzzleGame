#pragma once

#include <algorithm>
#include <cstdlib>
#include <vector>

template<int BOARD_SIZE> class Board;

extern Board<3> g_board3;
extern Board<4> g_board4;
extern Board<5> g_board5;

template<typename DATA_TYPE>
class Allocator
{
	static const int ALLOC_COUNT = 1024;
	std::vector<DATA_TYPE*> m_pool;
	std::vector<DATA_TYPE*> m_recycle;
	int m_curElement;

public:

	Allocator() :m_curElement(ALLOC_COUNT) {}

	~Allocator()
	{
		for (DATA_TYPE *p : m_pool) delete[] p;
	}

	DATA_TYPE* getNew()
	{
		if (!m_recycle.empty())
		{
			DATA_TYPE* res = m_recycle.back();
			m_recycle.pop_back();
			return res;
		}
		if (m_curElement == ALLOC_COUNT)
		{
			m_pool.push_back(new DATA_TYPE[ALLOC_COUNT]);
			m_curElement = 0;
		}
		return m_pool.back() + m_curElement++;
	}

	void recycle(DATA_TYPE *p)
	{
		m_recycle.push_back(p);
	}
};

struct PosInfo
{
	int row;
	int col;
};

template<int BOARD_SIZE>
class Board
{
	int m_board[BOARD_SIZE * BOARD_SIZE];
	int m_empty;

public:

	Board()
	{
		clear();
	}

	void clear()
	{
		m_empty = BOARD_SIZE * BOARD_SIZE - 1;
		for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
		{
			m_board[i] = i;
		}
	}

	// 获取当前棋盘位置pos上的图块的正确位置
	PosInfo getPiecePos(PosInfo pos)
	{
		const int num = m_board[pos.row * BOARD_SIZE + pos.col];
		return { num / BOARD_SIZE, num % BOARD_SIZE };
	}

	void random_shuffle()
	{
		// 拼图可解的充要条件：https://www.cnblogs.com/weiyinfu/p/5911340.html
		clear();
		int pos, rev = 0;
		for (int i = 0; i < BOARD_SIZE * BOARD_SIZE - 1; ++i)
		{
			pos = i + rand() % (BOARD_SIZE * BOARD_SIZE - i);
			if (pos == BOARD_SIZE * BOARD_SIZE - 1)
			{
				m_empty = i;
				//原理: rev ^= (BOARD_SIZE * BOARD_SIZE - 1 - 1) - i;
				rev ^= i ^ BOARD_SIZE;
				if (BOARD_SIZE % 2 == 0) rev ^= (BOARD_SIZE - 1) ^ (i / BOARD_SIZE);
			}
			else if (pos != i)
			{
				rev ^= 1;
			}
			std::swap(m_board[i], m_board[pos]);
		}
		if (rev & 1)
		{
			if (m_empty < 2) std::swap(m_board[2], m_board[3]);
			else std::swap(m_board[0], m_board[1]);
		}
	}
};