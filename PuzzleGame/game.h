#pragma once

#include <algorithm>
#include <cstdlib>

template<int BOARD_SIZE> class Board;

extern int g_boardSize;

extern Board<3> g_board3;
extern Board<4> g_board4;
extern Board<5> g_board5;

// 需保证异或1为逆操作
enum class MoveInfo
{
	MOVE_UP = 0,
	MOVE_DOWN = 1,
	MOVE_LEFT = 2,
	MOVE_RIGHT = 3
};

struct PosInfo
{
	int row;
	int col;
};

template <class BoardType> struct Status;

template <int BOARD_SIZE>
class Board
{
	int m_board[BOARD_SIZE * BOARD_SIZE];
	int m_empty;

	template <class BoardType>
	friend void Status<BoardType>::calcF();

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

	bool click(PosInfo pos)
	{
		if (pos.row < 0 || pos.row >= BOARD_SIZE || pos.col < 0 || pos.col >= BOARD_SIZE) return false;
		int erow = m_empty / BOARD_SIZE;
		int ecol = m_empty % BOARD_SIZE;
		if (erow == pos.row)
		{
			if (ecol == pos.col) return false;
			const int base = erow * BOARD_SIZE;
			if (ecol < pos.col)
			{
				for (int i = ecol; i < pos.col; ++i)
				{
					std::swap(m_board[base + i], m_board[base + i + 1]);
				}
			}
			else
			{
				for (int i = ecol; i > pos.col; --i)
				{
					std::swap(m_board[base + i], m_board[base + i - 1]);
				}
			}
			m_empty = base + pos.col;
			return true;
		}
		else if (ecol == pos.col)
		{
			if (erow == pos.row) return false;
			if (erow < pos.row)
			{
				for (int i = erow; i < pos.row; ++i)
				{
					std::swap(m_board[i * BOARD_SIZE + ecol], m_board[(i + 1) * BOARD_SIZE + ecol]);
				}
			}
			else
			{
				for (int i = erow; i > pos.row; --i)
				{
					std::swap(m_board[i * BOARD_SIZE + ecol], m_board[(i - 1) * BOARD_SIZE + ecol]);
				}
			}
			m_empty = pos.row * BOARD_SIZE + ecol;
			return true;
		}
		return false;
	}

	int getEmpty() const
	{
		return m_empty;
	}

	// 获取当前棋盘位置pos上的图块的正确位置
	PosInfo getPiecePos(PosInfo pos) const
	{
		const int num = m_board[pos.row * BOARD_SIZE + pos.col];
		return { num / BOARD_SIZE, num % BOARD_SIZE };
	}

	unsigned long long hash() const
	{
		unsigned long long res = 0;
		for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
		{
			res = res * 33 + m_board[i];
		}
		return res;
	}

	bool isFinished() const
	{
		for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
		{
			if (m_board[i] != i) return false;
		}
		return true;
	}

	// 返回是否成功移动
	bool move(MoveInfo mov)
	{
		int nextEmpty;
		bool res;
		switch (mov)
		{
		case MoveInfo::MOVE_UP:
			nextEmpty = m_empty + BOARD_SIZE;
			res = nextEmpty < BOARD_SIZE * BOARD_SIZE;
			break;

		case MoveInfo::MOVE_DOWN:
			nextEmpty = m_empty - BOARD_SIZE;
			res = nextEmpty >= 0;
			break;

		case MoveInfo::MOVE_LEFT:
			nextEmpty = m_empty + 1;
			res = nextEmpty % BOARD_SIZE != 0;
			break;

		case MoveInfo::MOVE_RIGHT:
			nextEmpty = m_empty - 1;
			res = m_empty % BOARD_SIZE != 0;
			break;
		}
		if (res)
		{
			std::swap(m_board[m_empty], m_board[nextEmpty]);
			m_empty = nextEmpty;
		}
		return res;
	}

	void random_shuffle()
	{
		// 拼图可解的充要条件：https://www.cnblogs.com/weiyinfu/p/5911340.html
		clear();
		int pos, rev = 0;
		for (int i = 0; i < BOARD_SIZE * BOARD_SIZE - 1; ++i)
		{
			pos = i + rand() % (BOARD_SIZE * BOARD_SIZE - i);
			if (pos == BOARD_SIZE * BOARD_SIZE - 1 && m_empty == BOARD_SIZE * BOARD_SIZE - 1)
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