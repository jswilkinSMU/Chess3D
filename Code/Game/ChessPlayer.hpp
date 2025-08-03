#pragma once
#include "Engine/Core/Rgba8.h"
#include <string>
// -----------------------------------------------------------------------------
class ChessMatch;
// -----------------------------------------------------------------------------
class ChessPlayer
{
public:
	ChessPlayer(int playerIDX, Rgba8 playerColor, ChessMatch* chessMatch);
	~ChessPlayer();
	int GetPlayerIndex() const { return m_playerIndex; }
	Rgba8 GetPlayerColor() const { return m_playerColor; }

	void SetPlayerName(std::string name);
	std::string GetPlayerName() const;

private:
	ChessMatch* m_theChessMatch = nullptr;
	std::string m_playerName = "default";
	int m_playerIndex = 0;
	Rgba8 m_playerColor = Rgba8::WHITE;
};