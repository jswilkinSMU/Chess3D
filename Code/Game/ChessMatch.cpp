#include "Game/ChessMatch.hpp"
#include "Game/ChessPlayer.hpp"
#include "Game/Game.h"
#include "Game/GameCommon.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Networking/NetworkSystem.hpp"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Input/InputSystem.h"

ChessMatch::ChessMatch(Game* owner)
	:m_theGame(owner)
{
	// Load the Chess Piece Definitions
	ChessPieceDefinition::InitializeChessDefs();

	// Chess Match creates the board
	m_board = new ChessBoard(this);

	// Chess Match has the two players
	m_playerOne = new ChessPlayer(0, Rgba8::WHITE, this);
	m_playerTwo = new ChessPlayer(1, Rgba8::BLACK, this);

	m_initialClockTime = g_gameConfigBlackboard.GetValue("chessClockTimeSeconds", 0.f);
	m_playerOneTimeRemaining = m_initialClockTime;
	m_playerTwoTimeRemaining = m_initialClockTime;

	// Subscribe to events
	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCmd", Event_RemoteCmd);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessDisconnect", Event_ChessDisconnect);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessPlayerInfo", Event_ChessPlayerInfo);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessBegin", Event_ChessBegin);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessValidate", Event_ChessValidate);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessMove", Event_ChessMove);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessResign", Event_ChessResign);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessOfferDraw", Event_ChessOfferDraw);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessAcceptDraw", Event_ChessAcceptDraw);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessRejectDraw", Event_ChessRejectDraw);
	g_theEventSystem->SubscribeEventCallbackFunction("SaveGame", Event_SaveChessGame);
	g_theEventSystem->SubscribeEventCallbackFunction("LoadGame", Event_LoadChessGame);

	// DevControls
	g_theDevConsole->AddLine(Rgba8::ORANGE, "===================================");
	g_theDevConsole->AddLine(Rgba8::ORANGE, "Player #0 Green, it's your turn!");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "GameState is: First Player's turn");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "   ABCDEFGH   ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "  +--------+  ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 8|rnbqkbnr|8 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 7|pppppppp|7 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 6|........|6 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 5|........|5 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 4|........|4 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 3|........|3 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 2|pppppppp|2 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " 1|rnbqkbnr|1 ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "  +--------+  ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "   ABCDEFGH   ");
}

ChessMatch::~ChessMatch()
{
	// Chess Match destroys the board
	delete m_board;
	m_board = nullptr;

	// ChessMatch destroys the players
	delete m_playerOne;
	m_playerOne = nullptr;
	delete m_playerTwo;
	m_playerTwo = nullptr;
}

void ChessMatch::Update(float deltaseconds)
{
	if (m_board != nullptr)
	{
		m_board->Update(deltaseconds);
	}

	DebugKeyPresses();

	if (m_replayMode)
	{
		ReplayRecordedMatch(deltaseconds);
	}

	if (m_chessClockActive)
	{
		UpdateChessClock(deltaseconds);
	}
}

void ChessMatch::DebugKeyPresses()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_lightDebugDraw = !m_lightDebugDraw;
	}
	if (g_theInput->WasKeyJustPressed('R'))
	{
		m_replayMode = !m_replayMode;
	}
	if (g_theInput->WasKeyJustPressed('C'))
	{
		m_chessClockActive = !m_chessClockActive;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		RewindOneMove();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		ForwardOneMove();
	}
}

void ChessMatch::UpdateChessClock(float deltaseconds)
{
	int currentPlayerIndex = m_playerTurnIndex % 2;
	if (!m_replayMode)
	{
		if (currentPlayerIndex == 0)
		{
			m_playerOneTimeRemaining -= deltaseconds;
			if (m_playerOneTimeRemaining <= 0.f)
			{
				AutoResignPlayer(0);
			}
		}
		else
		{
			m_playerTwoTimeRemaining -= deltaseconds;
			if (m_playerTwoTimeRemaining <= 0.f)
			{
				AutoResignPlayer(1);
			}
		}
	}
}

void ChessMatch::AutoResignPlayer(int playerIndex)
{
	int winningPlayer = (playerIndex == 0) ? 1 : 0;
	std::string message = Stringf("Player %d has resigned. Player %d wins!.", playerIndex, winningPlayer);
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, message);
}

void ChessMatch::RenderClockText() const
{
	std::string playerOneTimeText = Stringf("Player 1 Time: %.1f", m_playerOneTimeRemaining);
	std::string playerTwoTimeText = Stringf("Player 2 Time: %.1f", m_playerTwoTimeRemaining);

	AABB2 screenBox = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	DebugAddScreenText(playerOneTimeText.c_str(), screenBox, 15.f, Vec2(0.98f, 0.97f), 0.f);
	DebugAddScreenText(playerTwoTimeText.c_str(), screenBox, 15.f, Vec2(0.98f, 0.0f), 0.f);
}

void ChessMatch::Render() const
{
	Vec3 sunDirection = m_sunDirection;

	LightingConstants lightingConstants = { };
	lightingConstants.m_sunDirection = sunDirection;
	lightingConstants.m_ambientIntensity = m_ambientIntensity;
	lightingConstants.m_sunIntensity = m_sunIntensity;
	lightingConstants.NumPointLights = 0;
	lightingConstants.NumSpotLights = 1;
	Rgba8 pointLightColor = Rgba8::BLUE;

	// PointLight Testing
	pointLightColor.GetAsFloats(lightingConstants.PointLights[0].Color);
	lightingConstants.PointLights[0].Position = Vec4(4.f, 4.f, 0.3f, 1.f);

	// SpotLight Testing
	Rgba8 spotlightColor = Rgba8::WHITE;
	Vec3 spotlightPosition = Vec3(4.f, 4.f, 2.0f);
	Vec3 spotlightTarget = Vec3(4.f, 4.f, 0.f);

	Vec3 spotlightDirection = (spotlightTarget - spotlightPosition).GetNormalized();
	float innerAngleDegrees = 40.f;
	float outerAngleDegrees = 80.f;

	SpotLight& spotlight = lightingConstants.SpotLights[0];
	spotlight.m_position = Vec4(spotlightPosition.x, spotlightPosition.y, spotlightPosition.z, 1.f);
	spotlight.m_color = spotlightColor.GetAsVec4();

	spotlight.m_innerRadius = 3.0f;
	spotlight.m_outerRadius = 9.0f;
	spotlight.m_innerPenumbraDotThreshold = CosDegrees(innerAngleDegrees);
	spotlight.m_outerPenumbraDotThreshold = CosDegrees(outerAngleDegrees);
	spotlight.m_direction = spotlightDirection;
	
	g_theRenderer->SetLightingConstants(lightingConstants);
	m_board->Render();

	if (m_chessClockActive)
	{
		RenderClockText();
	}
}

bool ChessMatch::Event_RemoteCmd(EventArgs& args)
{
	std::string commandStr = args.GetValue("cmd", "");

	if (commandStr.empty())
	{
		return false;
	}

	std::map<std::string, std::string> const& valPairs = args.GetKeyValuePairs();
	std::map<std::string, std::string>::const_iterator found;

	for (found = valPairs.begin(); found != valPairs.end(); ++found)
	{
		std::string key = found->first;
		std::string val = found->second;

		if (key == "cmd")
		{
			continue;
		}

		commandStr += Stringf(" %s=%s", key.c_str(), val.c_str());
	}

	g_theDevConsole->Execute(commandStr);
	return true;
}

bool ChessMatch::Event_SaveChessGame(EventArgs& args)
{
	std::string filename = args.GetValue("file", "LastMatch.xml");
	std::string fullPath = "Data/SavedGames/" + filename;

	if (filename.empty())
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument: LoadGame file=fileName");
		return false;
	}

	g_theGame->m_theMatch->SaveGameToXmlFile(fullPath);
	return true;
}

bool ChessMatch::Event_LoadChessGame(EventArgs& args)
{
	std::string filename = args.GetValue("file", "");
	std::string fullPath = "Data/SavedGames/" + filename;

	if (filename.empty())
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument: LoadGame file=fileName");
		return false;
	}

	g_theGame->m_theMatch->LoadGameFromXmlFile(fullPath);
	return true;
}

bool ChessMatch::Event_ChessDisconnect(EventArgs& args)
{
	std::string reasonText = args.GetValue("reason", "");

	if (reasonText.empty())
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument ChessDisconnect reason=text");
		return false;
	}

	if (g_theNetwork->IsConnected())
	{
		if (g_theNetwork->IsServer())
		{
			for (uint64_t clientSocket : g_theNetwork->GetExternalClients())
			{
				g_theNetwork->Disconnect(clientSocket);
			}
		}
		else
		{
			g_theNetwork->Disconnect(g_theNetwork->GetMyClientSocket());
		}

		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Disconnected successfully!");
		return true;
	}

	g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "No active connection to disconnect from!");
	return false;
}

bool ChessMatch::Event_ChessPlayerInfo(EventArgs& args)
{
	std::string nameText = args.GetValue("name", "");
	int playerIndex = args.GetValue("player", -1);
	std::string remoteCommandText = args.GetValue("remote", "false");

	if (remoteCommandText == "true")
	{
		std::string remoteCmd = Stringf("ChessPlayerInfo name=%s player=%d", nameText.c_str(), playerIndex);
		g_theNetwork->SendStringToAll(remoteCmd);
	}

	if (playerIndex == 0)
	{
		g_theGame->m_theMatch->m_playerOne->SetPlayerName(nameText);
	}
	else if (playerIndex == 1)
	{
		g_theGame->m_theMatch->m_playerTwo->SetPlayerName(nameText);
	}

	return true;
}

bool ChessMatch::Event_ChessBegin(EventArgs& args)
{
	std::string firstPlayerText = args.GetValue("firstPlayer", "");
	std::string remoteCommandText = args.GetValue("remote", "false");

	g_theGame->DestroyMatch();
	g_theGame->InitializeChessMatch();

	if (remoteCommandText == "true")
	{
		std::string remoteCmd = "ChessBegin";
		g_theNetwork->SendStringToAll(remoteCmd);
	}

	return true;
}

bool ChessMatch::Event_ChessValidate(EventArgs& args)
{
	std::string remoteArgs	  = args.GetValue("remote", "");
	std::string boardArgs	  = args.GetValue("board", "");

	std::string gameState = GetGameStateAsString(g_theGame->GetCurrentGameState());
	std::string playerOneName = g_theGame->m_theMatch->m_playerOne->GetPlayerName();
	std::string playerTwoName = g_theGame->m_theMatch->m_playerTwo->GetPlayerName();
	int currentMove = g_theGame->m_theMatch->m_currentMoveIndex;

	g_theDevConsole->AddLine(Rgba8::YELLOW, "========= Chess Validate =========");
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Game State          : %s", gameState.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("PlayerOne Name      : %s", playerOneName.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("PlayerTwo Name      : %s", playerTwoName.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Move Number					    : %d", currentMove));
	g_theDevConsole->AddLine(Rgba8::CYAN, "Current Board Layout: ");
	g_theGame->m_theMatch->UpdateDevConsoleBoard();
	g_theDevConsole->AddLine(Rgba8::YELLOW, "==================================");

	if (remoteArgs == "true")
	{
		std::string remoteCmd = Stringf("ChessValidate state=%s player1=%s player2=%s move=%d board=%s", gameState.c_str(), playerOneName.c_str(), playerTwoName.c_str(), currentMove, boardArgs.c_str());
		g_theNetwork->SendStringToAll(remoteCmd);
	}

	return true;
}

bool ChessMatch::Event_ChessMove(EventArgs& args)
{
	// DevConsole command to move a piece is: “ChessMove from=e2 to=e4”.
	std::string chessPieceFrom = args.GetValue("from", "");
	std::string chessPieceTo = args.GetValue("to", "");

	// DevConsole command to use pawn promotion is: ChessMove from=e7 to=e8 promoteto=rook
	std::string pawnPromotion = args.GetValue("promoteTo", "");
	
	// DevConsole command to use dev cheat is: ChessMove from=a2 to=e5 teleport=true
	std::string chessPieceTeleport = args.GetValue("teleport", "");

	// DevConsole command to use to check if the ChessMove command really did originate from our computer
	// If remote=true, this command originated from a connected computer that is not me
	std::string remoteCommand = args.GetValue("remote", "false");

	// Check if arguments are missing
	//if (chessPieceFrom.empty())
	//{
	//	g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument: ChessMove from=value");
	//	return false;
	//}
	//if (chessPieceTo.empty())
	//{
	//	g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument: ChessMove to=value");
	//	return false;
	//}

	IntVec2 fromCoords = g_theGame->m_theMatch->m_board->GetCoordsForNotation(chessPieceFrom);
	IntVec2 toCoords = g_theGame->m_theMatch->m_board->GetCoordsForNotation(chessPieceTo);
	bool    isTeleporting = chessPieceTeleport == "true";
	bool	isRemote = remoteCommand == "true";

	// Check if we are remote
	if (isRemote)
	{
		// Our remote opponent is initiating a move
	}

	// Check if we are teleporting, if so we bypass move validation
	if (isTeleporting)
	{
		ChessPiece* piece = g_theGame->m_theMatch->m_board->GetChessPieceForCoords(fromCoords);
		if (!piece)
		{
			g_theAudio->StartSound(g_theGame->m_errorSound, false, 0.1f);
			g_theDevConsole->AddLine(Rgba8::RED, "No piece at source square.");
			return false;
		}

		g_theGame->m_theMatch->m_board->RemoveChessPieceAtCoords(fromCoords);
		piece->SetBoardPosition(toCoords);

		g_theGame->m_theMatch->UpdateDevConsoleBoard();
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("Chess Piece Moved to (%d, %d)", toCoords.x, toCoords.y));

		g_theGame->m_theMatch->m_playerTurnIndex += 1;
		int newCurrentPlayerIndex = (g_theGame->m_theMatch->m_playerTurnIndex % 2);
		if (g_theGame->m_currentCameraState != CameraState::FREEFLY)
		{
			if (newCurrentPlayerIndex == 0)
			{
				g_theGame->m_currentCameraState = CameraState::PLAYERONE_SPECTATING;
			}
			else if (newCurrentPlayerIndex == 1)
			{
				g_theGame->m_currentCameraState = CameraState::PLAYERTWO_SPECTATING;
			}
		}
		return true;
	}

	// Check if our move is valid
	ChessPiece* piece = g_theGame->m_theMatch->m_board->GetChessPieceForCoords(fromCoords);
	if (!piece)
	{
		g_theAudio->StartSound(g_theGame->m_errorSound, false, 0.1f);
		g_theDevConsole->AddLine(Rgba8::RED, "No piece at source square.");
		return false;
	}

	// Check if piece belongs to current player
	int currentPlayerIndex = (g_theGame->m_theMatch->m_playerTurnIndex % 2);
	if (piece->GetPlayerIndex() != currentPlayerIndex)
	{
		g_theAudio->StartSound(g_theGame->m_errorSound, false, 0.1f);
		g_theDevConsole->AddLine(Rgba8::RED, "You can't move your opponent's piece.");
		return false;
	}

	// Validate the move
	std::string moveError;
	ChessMoveResult result = piece->CheckMoveValidity(fromCoords, toCoords, moveError);
	if (!IsChessMoveValid(result))
	{
		g_theDevConsole->AddLine(Rgba8::RED, Stringf("%s", GetChessMoveResultString(result)));
		if (!moveError.empty()) 
		{
			g_theDevConsole->AddLine(Rgba8::YELLOW, moveError);
		}
		return false;
	}

	// Clear enpassant
	if (!(result == ChessMoveResult::VALID_MOVE_NORMAL && piece->GetDefinition()->m_chessPieceType == ChessPieceType::PAWN && abs(fromCoords.y - toCoords.y) == 2))
	{
		g_theGame->m_theMatch->m_board->m_enpassantTargetSquare = -IntVec2::ONE;
	}

	// Check for castling
	if (result == ChessMoveResult::VALID_CASTLE_KINGSIDE || result == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
	{
		IntVec2 rookFromCoords;
		IntVec2 rookToCoords;

		if (result == ChessMoveResult::VALID_CASTLE_KINGSIDE)
		{
			rookFromCoords = IntVec2(7, fromCoords.y);
			rookToCoords = IntVec2(toCoords.x - 1, fromCoords.y);
		}
		else 
		{
			rookFromCoords = IntVec2(0, fromCoords.y);
			rookToCoords = IntVec2(toCoords.x + 1, fromCoords.y);
		}

		ChessPiece* rook = g_theGame->m_theMatch->m_board->GetChessPieceForCoords(rookFromCoords);
		g_theGame->m_theMatch->m_board->RemoveChessPieceAtCoords(rookFromCoords);
		rook->SetBoardPosition(rookToCoords);
	}

	ChessPiece* capturedPiece = g_theGame->m_theMatch->m_board->GetChessPieceForCoords(toCoords);
	if (capturedPiece)
	{
		g_theGame->m_theMatch->m_board->RemoveChessPiece(capturedPiece);
		g_theDevConsole->AddLine(Rgba8::ORANGE, "A piece was captured!");
	}
	g_theGame->m_theMatch->m_board->RemoveChessPieceAtCoords(fromCoords);
	piece->SetBoardPosition(toCoords);

	// Pawn Promotion
	if (result == ChessMoveResult::VALID_MOVE_PROMOTION)
	{
		if (pawnPromotion.empty())
		{
			g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Missing argument! Correct argument: ChessMove promoteTo=value");
			return false;
		}

		ChessPieceType newType;
		if (pawnPromotion == "queen")
		{
			newType = ChessPieceType::QUEEN;
		}
		else if (pawnPromotion == "rook")
		{
			newType = ChessPieceType::ROOK;
		}
		else if (pawnPromotion == "bishop")
		{
			newType = ChessPieceType::BISHOP;
		}
		else if (pawnPromotion == "knight")
		{
			newType = ChessPieceType::KNIGHT;
		}
		else
		{
			g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Invalid promotion type. Must promote to a queen, rook, bishop or knight.");
			return false;
		}

		// Remove the pawn
		g_theGame->m_theMatch->m_board->RemoveChessPiece(piece);

		// Create newly promoted piece
		ChessPieceDefinition const& def = ChessPieceDefinition::GetChessPieceDef(newType);
		ChessPiece* promotedPiece = new ChessPiece(def, currentPlayerIndex, g_theGame->m_theMatch->m_board);
		promotedPiece->SetBoardPosition(toCoords);

		// Add our newly promoted piece to board 
		g_theGame->m_theMatch->m_board->AddChessPiece(promotedPiece);
	}

	// Play valid move audio
	if (result == ChessMoveResult::VALID_MOVE_NORMAL)
	{
		g_theAudio->StartSound(g_theGame->m_chessSlideSound, false, 5.f);
	}

	// Record our chess moves
	if (!g_theGame->m_theMatch->m_isReplayingMove)
	{
		std::string moveCommand = "ChessMove from=" + chessPieceFrom + " to=" + chessPieceTo;
		if (!pawnPromotion.empty()) 
		{
			moveCommand += " promoteTo=" + pawnPromotion;
		}
		if (chessPieceTeleport == "true") 
		{
			moveCommand += " teleport=true";
		}

		if (g_theGame->m_theMatch->m_currentMoveIndex < static_cast<int>(g_theGame->m_theMatch->m_moveHistory.size()) - 1) 
		{
			g_theGame->m_theMatch->m_moveHistory.erase(g_theGame->m_theMatch->m_moveHistory.begin() + g_theGame->m_theMatch->m_currentMoveIndex + 1, g_theGame->m_theMatch->m_moveHistory.end());
		}

		g_theGame->m_theMatch->m_moveHistory.push_back(moveCommand);
		g_theGame->m_theMatch->m_currentMoveIndex = static_cast<int>(g_theGame->m_theMatch->m_moveHistory.size()) - 1;
	}

	g_theGame->m_theMatch->UpdateDevConsoleBoard();
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("Chess Piece Moved to (%d, %d)", toCoords.x, toCoords.y));

	// Switch turns
	g_theGame->m_theMatch->m_playerTurnIndex += 1;
	int newCurrentPlayerIndex = (g_theGame->m_theMatch->m_playerTurnIndex % 2);
	if (g_theGame->m_currentCameraState != CameraState::FREEFLY)
	{
		if (newCurrentPlayerIndex == 0)
		{
			g_theGame->m_currentCameraState = CameraState::PLAYERONE_SPECTATING;
		}
		else if (newCurrentPlayerIndex == 1)
		{
			g_theGame->m_currentCameraState = CameraState::PLAYERTWO_SPECTATING;
		}
	}

	ChessPiece* opponentKing = nullptr;
	int opponentIndex = (currentPlayerIndex == 0) ? 1 : 0;
	for (ChessPiece* chessPiece : g_theGame->m_theMatch->m_board->m_chessPieces)
	{
		if (chessPiece && chessPiece->GetPlayerIndex() == opponentIndex && chessPiece->GetDefinition()->m_chessPieceType == ChessPieceType::KING)
		{
			opponentKing = chessPiece;
			break;
		}
	}

	std::string congratsText = Stringf("Congratulations! Player (%d) wins", currentPlayerIndex);

	if (opponentKing == nullptr)
	{
		g_theDevConsole->AddLine(Rgba8::GREEN, congratsText);

		// Save to XML
		std::string saveFilePath = "Data/SavedGames/CompletedMatch.xml";
		g_theGame->m_theMatch->SaveGameToXmlFile(saveFilePath);
	}

	// This will send our command to other connected people
	if (isRemote) 
	{
		std::string remoteCmd = "ChessMove from=" + chessPieceFrom + " to=" + chessPieceTo;
		if (!pawnPromotion.empty()) 
		{
			remoteCmd += " promoteTo=" + pawnPromotion;
		}
		if (chessPieceTeleport == "true") 
		{
			remoteCmd += " teleport=true";
		}
		g_theNetwork->SendStringToAll(remoteCmd);
	}

	return true;
}

void ChessMatch::RewindOneMove() 
{
	if (m_currentMoveIndex <= 0 || m_moveHistory.empty())
	{
		return;
	}

	m_currentMoveIndex -= 1;
	ResetBoardToInitialState();

	for (int moveIndex = 0; moveIndex <= m_currentMoveIndex; ++moveIndex) 
	{
		PlayMove(m_moveHistory[moveIndex]);
	}

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Rewound to move %d", m_currentMoveIndex));
}

void ChessMatch::ForwardOneMove()
{
	if (m_currentMoveIndex <= 0 || m_moveHistory.empty())
	{
		return;
	}

	m_currentMoveIndex += 1;
	ResetBoardToInitialState();

	for (int moveIndex = 0; moveIndex <= m_currentMoveIndex; ++moveIndex)
	{
		PlayMove(m_moveHistory[moveIndex]);
	}

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Forward to move %d", m_currentMoveIndex));
}

bool ChessMatch::Event_ChessResign(EventArgs& args)
{
	int playerIndex = args.GetValue("player", -1);

	// Check if we are actually a valid player
	if (playerIndex != 0 && playerIndex != 1) 
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "ChessResign requires 'player=0' or 'player=1'");
		return false;
	}

	g_theGame->m_theMatch->AutoResignPlayer(playerIndex);
	return true;
}

bool ChessMatch::Event_ChessOfferDraw(EventArgs& args)
{
	std::string remoteArgs = args.GetValue("remote", "false");

	g_theGame->m_theMatch->m_hasPlayerOfferedDraw = true;

	if (remoteArgs == "true")
	{
		std::string remoteCommand = "Your opponent has offered a draw! \n Accept the draw with ChessAcceptDraw or reject the draw with ChessRejectDraw.";
		g_theNetwork->SendStringToAll(remoteCommand);
		return true;
	}

	return false;
}

bool ChessMatch::Event_ChessAcceptDraw(EventArgs& args)
{
	std::string remoteArgs = args.GetValue("remote", "false");

	g_theGame->m_theMatch->m_hasPlayerOfferedDraw = false;
	g_theGame->EnterState(GameState::FINISHED_MATCH);

	if (remoteArgs == "true")
	{
		std::string remoteCommand = "Accepted the draw offer, chess match will end!";
		g_theNetwork->SendStringToAll(remoteCommand);
		g_theGame->EnterState(GameState::ATTRACT);
		return true;
	}

	return false;
}

bool ChessMatch::Event_ChessRejectDraw(EventArgs& args)
{
	std::string remoteArgs = args.GetValue("remote", "false");

	g_theGame->m_theMatch->m_hasPlayerOfferedDraw = false;

	if (remoteArgs == "true")
	{
		std::string remoteCommand = "Rejected the draw offer, chess match will keep playing!";
		g_theNetwork->SendStringToAll(remoteCommand);
		return true;
	}

	return false;
}

void ChessMatch::SaveGameToXmlFile(std::string const& filePath) const
{
	XmlDocument xmlDocument;

	XmlElement* root = xmlDocument.NewElement("ChessMatch");
	xmlDocument.InsertFirstChild(root);

	// Save each move
	for (int moveIndex = 0; moveIndex < static_cast<int>(m_moveHistory.size()); ++moveIndex)
	{
		std::string const& moveCommand = m_moveHistory[moveIndex];

		XmlElement* moveElement = xmlDocument.NewElement("Move");
		moveElement->SetText(moveCommand.c_str());

		root->InsertEndChild(moveElement);
	}

	// Save to file
	XmlError result = xmlDocument.SaveFile(filePath.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Failed to save XML file.");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Match saved to XML: " + filePath);
	}
}

void ChessMatch::LoadGameFromXmlFile(std::string const& filePath)
{
	XmlDocument xmlDocument;
	XmlError result = xmlDocument.LoadFile(filePath.c_str());

	if (result != tinyxml2::XML_SUCCESS) 
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Failed to load XML file.");
	}

	XmlElement* root = xmlDocument.RootElement();
	if (root == nullptr) 
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Invalid game data.");
	}

	// Clearing out history in case we have recorded any moves
	m_moveHistory.clear();

	for (XmlElement* moveElement = root->FirstChildElement("Move"); moveElement != nullptr; moveElement = moveElement->NextSiblingElement("Move"))
	{
		char const* moveCommand = moveElement->GetText();
		if (moveCommand != nullptr) 
		{
			m_moveHistory.push_back(moveCommand);
			g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("Loaded %d moves from XML file.", m_moveHistory.size()));
		}
	}

	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Game loaded successfully.");
}

void ChessMatch::ReplayRecordedMatch(float deltaSeconds)
{
	if (m_moveHistory.empty())
	{
		return;
	}

	float timeDelay = g_gameConfigBlackboard.GetValue("secondsBetweenPlaybackMoves", 1.f);
	if (timeDelay == -1)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
		{
			PlayMove(m_moveHistory[m_currentMoveIndex]);
			m_timeSinceLastMove = 0.f;
			m_currentMoveIndex += 1;
		}
	}
	else
	{
		m_timeSinceLastMove += deltaSeconds;
		if (m_timeSinceLastMove >= timeDelay && m_currentMoveIndex < static_cast<int>(m_moveHistory.size()))
		{
			PlayMove(m_moveHistory[m_currentMoveIndex]);
			m_timeSinceLastMove = 0.f;
			m_currentMoveIndex += 1;
		}
	}
}

void ChessMatch::PlayMove(std::string const& moveCommand)
{
	m_isReplayingMove = true;
	g_theDevConsole->Execute(moveCommand);
	m_isReplayingMove = false;
}

void ChessMatch::UpdateDevConsoleBoard()
{
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "  ABCDEFGH  ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " +--------+ ");
	for (int y = 7; y >= 0; --y)
	{
		std::string row = std::to_string(y + 1) + "|";
		for (int x = 0; x < CHESS_BOARD_COLUMNS; ++x)
		{
			ChessPiece* piece = m_board->GetChessPieceForCoords(x, y);
			if (piece)
			{
				row += piece->GetDefinition()->m_glyph;
			}
			else
			{
				row += '.';
			}
		}
		row += "|" + std::to_string(y + 1);
		g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, row);
	}
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, " +--------+ ");
	g_theDevConsole->AddLine(Rgba8::LIGHTBLUE, "  ABCDEFGH  ");

	if (m_chessClockActive)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Player 0 Time: %.1fs", m_playerOneTimeRemaining));
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Player 1 Time: %.1fs", m_playerTwoTimeRemaining));
	}
}

void ChessMatch::ResetBoardToInitialState()
{
	delete m_board;
	m_board = new ChessBoard(this); 
	m_playerTurnIndex = 0;
	m_timeSinceLastMove = 0.f;
}

