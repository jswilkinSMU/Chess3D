#include "Game/ChessPlayer.hpp"

ChessPlayer::ChessPlayer(int playerIDX, Rgba8 playerColor, ChessMatch* chessMatch)
	:m_playerIndex(playerIDX), 
	 m_playerColor(playerColor), 
	 m_theChessMatch(chessMatch)
{
}

ChessPlayer::~ChessPlayer()
{
}

void ChessPlayer::SetPlayerName(std::string name)
{
	m_playerName = name;
}

std::string ChessPlayer::GetPlayerName() const
{
	return m_playerName;
}
