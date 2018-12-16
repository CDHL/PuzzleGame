#include "stdafx.h"

#include "game.h"

#include <algorithm>
#include <cstdlib>

template<int BOARD_SIZE>
Board<BOARD_SIZE>::Board()
{
	clear();
}

template<int BOARD_SIZE>
void Board<BOARD_SIZE>::clear()
{
	m_zero = BOARD_SIZE * BOARD_SIZE - 1;
	m_board[m_zero] = 0;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE - 1; ++i)
	{
		m_board[i] = i + 1;
	}
}

template<int BOARD_SIZE>
void Board<BOARD_SIZE>::random_shuffle()
{
	clear();
	int pos, rev = 0;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE - 1; ++i)
	{
		pos = i + rand() % (BOARD_SIZE * BOARD_SIZE - i);
		if (pos == BOARD_SIZE * BOARD_SIZE - 1)
		{
			m_zero = i;
			//Ô­Àí: rev ^= (BOARD_SIZE * BOARD_SIZE - 1 - 1) - i;
			rev ^= i ^ BOARD_SIZE;
			if(BOARD_SIZE % 2 == 0) rev ^= (BOARD_SIZE - 1) ^ (i / BOARD_SIZE);
		}
		else if (pos != i)
		{
			rev ^= 1;
		}
		std::swap(m_board[i], m_board[pos]);
	}
	if (rev & 1)
	{
		if (m_zero < 2) std::swap(m_board[2], m_board[3]);
		else std::swap(m_board[0], m_board[1]);
	}
}
