#pragma once
#include "Game/ChessBoard.hpp"
#include "Engine/Math/Vec3.h"
#include "Engine/Core/EventSystem.hpp"
// -----------------------------------------------------------------------------
class Game;
class ChessPlayer;
// -----------------------------------------------------------------------------
class ChessMatch
{
public:
	ChessMatch(Game* owner);
	~ChessMatch();

	// Updating
	void Update(float deltaseconds);
	void DebugKeyPresses();
	void UpdateDevConsoleBoard();
	void ResetBoardToInitialState();

	// Rendering
	void Render() const;

	// Local events
	static bool Event_RemoteCmd(EventArgs& args);
	static bool Event_SaveChessGame(EventArgs& args);
	static bool Event_LoadChessGame(EventArgs& args);

	// Remote events
	static bool Event_ChessDisconnect(EventArgs& args);
	static bool Event_ChessPlayerInfo(EventArgs& args);
	static bool Event_ChessBegin(EventArgs& args);
	static bool Event_ChessValidate(EventArgs& args);
	static bool Event_ChessMove(EventArgs& args);
	static bool Event_ChessResign(EventArgs& args);
	static bool Event_ChessOfferDraw(EventArgs& args);
	static bool Event_ChessAcceptDraw(EventArgs& args);
	static bool Event_ChessRejectDraw(EventArgs& args);

	// Saving the game to xml
	void SaveGameToXmlFile(std::string const& filePath) const;

	// Loading a recorded game from xml
	void LoadGameFromXmlFile(std::string const& filePath);
	void ReplayRecordedMatch(float deltaSeconds);
	void PlayMove(std::string const& moveCommand);

	// Chess Clock
	void UpdateChessClock(float deltaseconds);
	void AutoResignPlayer(int playerIndex);
	void RenderClockText() const;

	// Rewinding Timeline
	void RewindOneMove();
	void ForwardOneMove();

public:
	int m_playerTurnIndex = 0;
	ChessBoard* m_board = nullptr;
	std::vector<std::string> m_moveHistory;

private:
	Game* m_theGame = nullptr;
	ChessPlayer* m_playerOne = nullptr;
	ChessPlayer* m_playerTwo = nullptr;
	bool m_hasPlayerOfferedDraw = false;

	// Lighting
	Vec3 m_sunDirection = Vec3(3.f, 1.f, -2.f);
	float m_sunIntensity = 0.15f;
	float m_ambientIntensity = 0.05f;

	// Debugging
	bool  m_lightDebugDraw = false;
	bool  m_replayMode = false;
	bool  m_chessClockActive = false;

	// Timing
	float m_timeSinceLastMove = 0.0f;
	int   m_currentMoveIndex = 0;
	float m_playerOneTimeRemaining = 0.f;
	float m_playerTwoTimeRemaining = 0.f;
	float m_initialClockTime = 300.f;
	//int m_currentMoveIndex = -1;
	bool m_isReplayingMove = false;
};