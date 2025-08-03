#include "Game/Game.h"
#include "Game/GameCommon.h"
#include "Game/App.h"
#include "Game/ChessMatch.hpp"
#include "Game/ChessObject.hpp"
#include "Game/ChessBoard.hpp"
#include "Game/ChessPiece.hpp"

#include "Engine/Input/InputSystem.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Networking/NetworkSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.h"
#include "Engine/Core/Vertex_PCU.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexUtils.h"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/AABB3.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Elements/UIButton.hpp"
#include "Engine/UI/Elements/UIBorder.hpp"

Game::Game(App* owner)
	: m_app(owner)
{
	// Get sound from game config
	m_gameMusicPath = g_gameConfigBlackboard.GetValue("gameMusic", "default");
	m_clickSoundPath = g_gameConfigBlackboard.GetValue("buttonClickSound", "default");
	m_musicVolume = g_gameConfigBlackboard.GetValue("musicVolume", 0.f);

	m_gameMusic = g_theAudio->CreateOrGetSound(m_gameMusicPath);
	m_clickSound = g_theAudio->CreateOrGetSound(m_clickSoundPath);
	m_gameStartSound = g_theAudio->CreateOrGetSound("Data/Audio/BoardStart.mp3");
	m_clickOffSound = g_theAudio->CreateOrGetSound("Data/Audio/ClickOff.wav");
	m_errorSound = g_theAudio->CreateOrGetSound("Data/Audio/MoveError.wav");
	m_chessSlideSound = g_theAudio->CreateOrGetSound("Data/Audio/ChessPieceMove.mp3");
	m_gameMusicPlayback = m_gameMusic;
}

Game::~Game()
{
}

void Game::StartUp()
{
	// Write control interface into devconsole
	g_theDevConsole->AddLine(Rgba8::CYAN, "Welcome to Chess3D!");
	g_theDevConsole->AddLine(Rgba8::SEAWEED, "----------------------------------------------------------------------");
	g_theDevConsole->AddLine(Rgba8::CYAN, "CONTROLS:");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "ESC   - Quits the game");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "SPACE - Start game");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "SHIFT - Increase speed by factor of 10.");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "A/D   - Move left/right");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "W/S   - Move forward/backward");
	g_theDevConsole->AddLine(Rgba8::LIGHTYELLOW, "Z/C   - Move down/up");
	g_theDevConsole->AddLine(Rgba8::SEAWEED, "----------------------------------------------------------------------");

	m_font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	EnterState(GameState::ATTRACT);

	g_theEventSystem->SubscribeEventCallbackFunction("ChessServerInfo", Event_ChessServerInfo);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessListen", Event_ChessListen);
	g_theEventSystem->SubscribeEventCallbackFunction("ChessConnect", Event_Chess_Connect);
}

void Game::InitializeChessMatch()
{
	m_theMatch = new ChessMatch(this);
}

void Game::SetupAttractButtons()
{
	AABB2 enterButtonBounds = AABB2(600.f, 420.f, 1000.f, 480.f);
	AABB2 exitButtonBounds = AABB2(600.f, 320.f, 1000.f, 380.f);
	AABB2 startServerButtonBounds = AABB2(600.f, 220.f, 780.f, 280.f);
	AABB2 startClientButtonBounds = AABB2(820.f, 220.f, 1000.f, 280.f);

	UIButton* startButton = new UIButton("EnterButton", enterButtonBounds, "Enter", m_font);
	startButton->SetButtonBackgroundColor(Rgba8::SEAWEED);
	startButton->SetButtonHoverColor(Rgba8(20, 60, 20, 120));
	startButton->SetOnClickCallback([this]()
	{
		g_theAudio->StartSound(m_gameStartSound, false, 0.4f);
		EnterState(GameState::GAME_PLAYING);
	});
	g_theUISystem->AddElement(startButton);

	UIBorder* startButtonBorder = new UIBorder("StartBorder", enterButtonBounds, Rgba8::BLACK, 2.f);
	g_theUISystem->AddElement(startButtonBorder);

	UIButton* exitButton = new UIButton("ExitButton", exitButtonBounds, "Exit", m_font);
	exitButton->SetButtonBackgroundColor(Rgba8::DARKRED);
	exitButton->SetButtonHoverColor(Rgba8(139, 0, 0, 120));
	exitButton->SetOnClickCallback([]()
	{
		g_theEventSystem->FireEvent("Quit");
	});
	g_theUISystem->AddElement(exitButton);

	UIBorder* exitButtonBorder = new UIBorder("ExitBorder", exitButtonBounds, Rgba8::BLACK, 2.f);
	g_theUISystem->AddElement(exitButtonBorder);

	UIButton* startServerButton = new UIButton("StartServerButton", startServerButtonBounds, "Start\nServer", m_font);
	startServerButton->SetButtonBackgroundColor(Rgba8::SEAWEED);
	startServerButton->SetButtonHoverColor(Rgba8(20, 60, 20, 120));
	startServerButton->SetOnClickCallback([this]()
	{
		// Starting an existing server
		g_theAudio->StartSound(m_clickSound, false, m_musicVolume);
		g_theEventSystem->FireEvent("ChessListen");
	});
	g_theUISystem->AddElement(startServerButton);

	UIBorder* startServerBorder = new UIBorder("StartServerBorder", startServerButtonBounds, Rgba8::BLACK, 2.f);
	g_theUISystem->AddElement(startServerBorder);

	UIButton* startClientButton = new UIButton("StartClientButton", startClientButtonBounds, "Start\nClient", m_font);
	startClientButton->SetButtonBackgroundColor(Rgba8::SEAWEED);
	startClientButton->SetButtonHoverColor(Rgba8(20, 60, 20, 120));
	startClientButton->SetOnClickCallback([this]()
	{
		// Starting an existing client
		g_theAudio->StartSound(m_clickSound, false, m_musicVolume);
		g_theEventSystem->FireEvent("ChessConnect");
	});
	g_theUISystem->AddElement(startClientButton);

	UIBorder* startClientBorder = new UIBorder("StartClientBorder", startClientButtonBounds, Rgba8::BLACK, 2.f);
	g_theUISystem->AddElement(startClientBorder);

	UIBorder* attractBorder = new UIBorder("AttractBorder", AABB2(Vec2(500.f, 650.f), Vec2(1100.f, 150.f)), Rgba8::WHITE, 4.f);
	attractBorder->SetBorderStyle(BorderStyle::ROUNDED);
	attractBorder->SetGapLength(7.f);
	attractBorder->SetDashLength(10.f);
	g_theUISystem->AddElement(attractBorder);

	UIBorder* titleBorder = new UIBorder("TitleBorder", AABB2(Vec2(550.f, 550.f), Vec2(1050.f, 550.f)), Rgba8::WHITE, 4.f);
	titleBorder->SetBorderStyle(BorderStyle::DASHED);
	titleBorder->SetGapLength(7.f);
	titleBorder->SetDashLength(10.f);
	g_theUISystem->AddElement(titleBorder);
}

void Game::Update()
{
	// Setting clock time variables
	double deltaSeconds = m_gameClock.GetDeltaSeconds();

	// Set debug text
	std::string debugText = Stringf("Debug Mode [%d]: %s", m_debugInt, GetDebugRenderModeDesc(m_debugInt));
	DebugAddScreenText(debugText, AABB2(0.f, 0.f, SCREEN_SIZE_X, SCREEN_SIZE_Y), 10.f, Vec2(0.0f, 0.97f), 0.f);
	g_theRenderer->SetPerFrameConstants(m_debugInt, 0.f);

	UpdateUIPresses(static_cast<float>(deltaSeconds));
	g_theNetwork->ProcessIncomingMessages();

	if (m_theMatch != nullptr)
	{
		m_theMatch->Update(static_cast<float>(deltaSeconds));
		UpdateRaycast();
		UpdateHighlighted();
		UpdateSelected();
	}

	AdjustForPauseAndTimeDistortion(static_cast<float>(deltaSeconds));
	KeyInputPresses();
	UpdateCameras(static_cast<float>(deltaSeconds));
}

void Game::UpdateUIPresses(float deltaSeconds)
{
	Vec2 clientPos = g_theInput->GetCursorClientPosition();
	Vec2 worldPos = m_screenCamera.GetClientToWorld(clientPos, g_theWindow->GetClientDimensions());
	g_theUISystem->Update(deltaSeconds, worldPos);

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		g_theUISystem->HandleMouseClick(worldPos);
	}
}

void Game::Render() const
{
	if (m_currentState == GameState::ATTRACT)
	{
		g_theRenderer->BeginCamera(m_screenCamera);
		g_theUISystem->Render();
		RenderAttractMode();
		g_theRenderer->EndCamera(m_screenCamera);
	}
	if (m_currentState == GameState::GAME_PLAYING)
	{
		g_theRenderer->BeginCamera(m_gameWorldCamera);
		g_theRenderer->ClearScreen(Rgba8(70, 70, 70, 255));
		if (m_currentCameraState == CameraState::FREEFLY)
		{
			DrawBasis();
		}
		RenderMatch();
		g_theRenderer->EndCamera(m_gameWorldCamera);
		g_theRenderer->BindShader(nullptr);

		DebugRenderWorld(m_gameWorldCamera);
		DebugRenderScreen(m_screenCamera);
	}
}

void Game::Shutdown()
{
	// Destroy the match on shutdown
	DestroyMatch();
}

void Game::DestroyMatch()
{
	delete m_theMatch;
	m_theMatch = nullptr;
}

void Game::KeyInputPresses()
{
	// Attract Mode
	if (g_theInput->WasKeyJustPressed(' '))
	{
		EnterState(GameState::GAME_PLAYING);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		EnterState(GameState::ATTRACT);
	}

	// Debug Visualization Keys
	DebugVisuals();
}

void Game::DebugVisuals()
{
	if (g_theInput->WasKeyJustPressed('0'))
	{
		m_debugInt = 0;
	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_debugInt = 1;
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_debugInt = 2;
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_debugInt = 3;
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		m_debugInt = 7;
	}
	if (g_theInput->WasKeyJustPressed('8'))
	{
		m_debugInt = 8;
	}
	if (g_theInput->WasKeyJustPressed('9'))
	{
		m_debugInt = 9;
	}

	if (g_theInput->WasKeyJustPressed('K'))
	{
		m_debugInt = 10;
	}
	if (g_theInput->WasKeyJustPressed('L'))
	{
		m_debugInt = 11;
	}

	// Debug Tangent/Bitangent/Normal
	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_debugInt = 4;
	}
	if (g_theInput->WasKeyJustPressed('B'))
	{
		m_debugInt = 5;
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		m_debugInt = 6;
	}

	// Debug Specular/Glossiness/Emissive
	// Need to enable Nums Lock to use numpad keys
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD0))
	{
		m_debugInt = 14;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD1))
	{
		m_debugInt = 15;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD2))
	{
		m_debugInt = 16;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD3))
	{
		m_debugInt = 17;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD4))
	{
		m_debugInt = 18;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD5))
	{
		m_debugInt = 19;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_NUMPAD6))
	{
		m_debugInt = 20;
	}

	// Raycast debug
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_isRaycastDebug = !m_isRaycastDebug;
	}
}

void Game::UpdateRaycast()
{
	if (m_currentCameraState == CameraState::PLAYERONE_SPECTATING || m_currentCameraState == CameraState::PLAYERTWO_SPECTATING)
	{
		return;
	}

	// First Reset
	m_lastRaycastResult.Reset();

	// Set Camera Forward as raycast start
	m_lastRaycastResult.m_rayStartPosition = m_cameraPosition;
	m_lastRaycastResult.m_rayFwdNormal = GetCameraFwdNormal();
	m_lastRaycastResult.m_rayLength = 20.f;

	// Check that we are in playing state
	if (m_currentState != GameState::GAME_PLAYING)
	{
		return;
	}

	// Then raycast vs board and pieces
	m_theMatch->m_board->RaycastVsBoardAndPieces(m_lastRaycastResult);
}

void Game::UpdateHighlighted()
{
	if (m_currentCameraState == CameraState::PLAYERONE_SPECTATING || m_currentCameraState == CameraState::PLAYERTWO_SPECTATING)
	{
		return;
	}

	m_highlightedSquare = -IntVec2::ONE;

	if (m_lastRaycastResult.m_coords != -IntVec2::ONE)
	{
		m_highlightedSquare = m_lastRaycastResult.m_coords;

		IntVec2 coords = m_lastRaycastResult.m_coords;
		float z = 0.005f;
		float squareSize = 1.f;

		Vec3 bottomLeft = Vec3(coords.x * squareSize, coords.y * squareSize, z);
		Vec3 bottomRight = Vec3((coords.x + 1) * squareSize, coords.y * squareSize, z);
		Vec3 topRight = Vec3((coords.x + 1) * squareSize, (coords.y + 1) * squareSize, z);
		Vec3 topLeft = Vec3(coords.x * squareSize, (coords.y + 1) * squareSize, z);

		bool isHoverValid = false;

		if (m_lastRaycastResult.m_didImpact)
		{
			if (m_selectedPiece == nullptr)
			{
				isHoverValid = true;
			}
			else
			{
				IntVec2 from = m_selectedCoords;
				IntVec2 to = m_lastRaycastResult.m_coords;
				std::string unusedError;

				ChessMoveResult result = m_selectedPiece->CheckMoveValidity(from, to, unusedError);
				isHoverValid = (result == ChessMoveResult::VALID_MOVE_NORMAL);
				if (isHoverValid)
				{
					DebugAddWorldQuad(bottomLeft, bottomRight, topRight, topLeft, 0.f, Rgba8::GREEN, Rgba8::GREEN);
				}
			}
			if (m_isRaycastDebug)
			{
				if (isHoverValid)
				{
					DebugAddWorldSphere(m_lastRaycastResult.m_impactPos, 0.05f, 0.f, Rgba8::WHITE, Rgba8::WHITE);
					DebugAddWorldWireQuad(bottomLeft, bottomRight, topRight, topLeft, 0.f, Rgba8::WHITE, Rgba8::WHITE);
				}

				if (m_lastRaycastResult.m_chessPiece)
				{
					Vec3 base = Vec3::ZERO, top = Vec3::ZERO;
					float radius = 0.f;
					m_lastRaycastResult.m_chessPiece->GetCylinderForRaycastDebug(base, top, radius);
					DebugAddWorldWireCylinder(base, top, radius, 0.f);
				}
			}
		}
	}
	if (m_selectedPiece != nullptr)
	{
		IntVec2 coords = m_selectedCoords;
		float z = 0.005f;
		float squareSize = 1.f;

		Vec3 bottomLeft = Vec3(coords.x * squareSize, coords.y * squareSize, z);
		Vec3 bottomRight = Vec3((coords.x + 1) * squareSize, coords.y * squareSize, z);
		Vec3 topRight = Vec3((coords.x + 1) * squareSize, (coords.y + 1) * squareSize, z);
		Vec3 topLeft = Vec3(coords.x * squareSize, (coords.y + 1) * squareSize, z);

		DebugAddWorldQuad(bottomLeft, bottomRight, topRight, topLeft, 0.f, Rgba8::GREEN, Rgba8::GREEN);
	}
}

void Game::UpdateSelected()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		if (m_selectedPiece != nullptr)
		{
			g_theAudio->StartSound(m_clickOffSound);
			m_selectedPiece = nullptr;
			m_selectedCoords = -IntVec2::ONE;
		}
		return;
	}

	if (!g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		return;
	}

	if (m_selectedPiece == nullptr)
	{
		if (m_lastRaycastResult.m_didImpact && m_lastRaycastResult.m_chessPiece != nullptr)
		{
			ChessPiece* clickedPiece = m_lastRaycastResult.m_chessPiece;
			int currentPlayerIndex = m_theMatch->m_playerTurnIndex % 2;

			if (clickedPiece->GetPlayerIndex() == currentPlayerIndex)
			{
				m_selectedPiece = clickedPiece;
				m_selectedCoords = clickedPiece->GetBoardPosition();
				g_theAudio->StartSound(m_clickSound, false, m_musicVolume);
			}
			else
			{
				g_theDevConsole->AddLine(Rgba8::RED, "Cannot select enemy piece.");
				g_theAudio->StartSound(m_errorSound, false, m_musicVolume);
			}
		}
	}
	else
	{
		if (m_lastRaycastResult.m_coords != -IntVec2::ONE)
		{
			std::string fromNotation = m_theMatch->m_board->GetNotationForCoords(m_selectedCoords);
			std::string toNotation = m_theMatch->m_board->GetNotationForCoords(m_lastRaycastResult.m_coords);
			std::string moveCommand = Stringf("ChessMove from=%s to=%s", fromNotation.c_str(), toNotation.c_str());
			g_theDevConsole->Execute(moveCommand);

			if (g_theInput->IsKeyDown(KEYCODE_CTRL))
			{
				std::string moveTeleportCmd = Stringf("ChessMove from=%s to=%s teleport=true", fromNotation.c_str(), toNotation.c_str());
				g_theDevConsole->Execute(moveTeleportCmd);
			}
		}
		m_selectedPiece = nullptr;
		m_selectedCoords = -IntVec2::ONE;
	}
}

ChessPiece* Game::GetSelectedPiece() const
{
	return m_selectedPiece;
}

void Game::AdjustForPauseAndTimeDistortion(float deltaSeconds) {

	UNUSED(deltaSeconds);

	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepSingleFrame();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_currentState == GameState::ATTRACT)
	{
		g_theEventSystem->FireEvent("Quit");
	}
}

void Game::UpdateCameras(float deltaSeconds)
{
	// Screen Camera
	m_screenCamera.SetOrthoView(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	CameraSwitch();

	// Game Camera
	Mat44 cameraToRender(Vec3::ZAXE, -Vec3::XAXE, Vec3::YAXE, Vec3::ZERO);
	m_gameWorldCamera.SetCameraToRenderTransform(cameraToRender);

	if (m_currentCameraState == CameraState::FREEFLY)
	{
		FreeFlyControls(deltaSeconds);

		m_cameraOrientation.m_pitchDegrees = GetClamped(m_cameraOrientation.m_pitchDegrees, -85.f, 85.f);
		m_cameraOrientation.m_rollDegrees = GetClamped(m_cameraOrientation.m_rollDegrees, -45.f, 45.f);

		m_gameWorldCamera.SetPositionAndOrientation(m_cameraPosition, m_cameraOrientation);

		m_gameWorldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
	}
	else if (m_currentCameraState == CameraState::PLAYERONE_SPECTATING)
	{
		m_cameraPosition = Vec3(5.f, -2.f, 5.f);
		m_cameraOrientation = EulerAngles(90.f, 45.f, 0.f);
		m_gameWorldCamera.SetPositionAndOrientation(m_cameraPosition, m_cameraOrientation);
		m_gameWorldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
	}
	else if (m_currentCameraState == CameraState::PLAYERTWO_SPECTATING)
	{
		m_cameraPosition = Vec3(5.f, 10.f, 5.f);
		m_cameraOrientation = EulerAngles(260.f, 45.f, 0.f);
		m_gameWorldCamera.SetPositionAndOrientation(m_cameraPosition, m_cameraOrientation);
		m_gameWorldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
	}
}

void Game::CameraSwitch()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4) && m_currentCameraState == CameraState::FREEFLY)
	{
		m_currentCameraState = CameraState::PLAYERONE_SPECTATING;
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_F4) && m_currentCameraState == CameraState::PLAYERONE_SPECTATING && m_theMatch->m_playerTurnIndex == 0)
	{
		m_currentCameraState = CameraState::FREEFLY;
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_F4) && m_currentCameraState == CameraState::PLAYERTWO_SPECTATING && m_theMatch->m_playerTurnIndex == 1)
	{
		m_currentCameraState = CameraState::FREEFLY;
	}
}

void Game::FreeFlyControls(float deltaSeconds)
{
	// Yaw and Pitch with mouse
	m_cameraOrientation.m_yawDegrees += 0.08f * g_theInput->GetCursorClientDelta().x;
	m_cameraOrientation.m_pitchDegrees -= 0.08f * g_theInput->GetCursorClientDelta().y;

	float movementSpeed = 2.f;
	// Increase speed by a factor of 10
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		movementSpeed *= 10.f;
	}

	// Move left or right
	if (g_theInput->IsKeyDown('A'))
	{
		m_cameraPosition += movementSpeed * m_cameraOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D() * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_cameraPosition += -movementSpeed * m_cameraOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D() * deltaSeconds;
	}

	// Move Forward and Backward
	if (g_theInput->IsKeyDown('W'))
	{
		m_cameraPosition += movementSpeed * m_cameraOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D() * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_cameraPosition += -movementSpeed * m_cameraOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D() * deltaSeconds;
	}

	// Move Up and Down
	if (g_theInput->IsKeyDown('Z'))
	{
		m_cameraPosition += -movementSpeed * Vec3::ZAXE * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('C'))
	{
		m_cameraPosition += movementSpeed * Vec3::ZAXE * deltaSeconds;
	}
}

Vec3 Game::GetCameraFwdNormal() const
{
	return Vec3::MakeFromPolarDegrees(m_cameraOrientation.m_pitchDegrees, m_cameraOrientation.m_yawDegrees);
}

void Game::RenderAttractMode() const
{
	std::vector<Vertex_PCU> textVerts;
	m_font->AddVertsForTextInBox2D(textVerts, "Chess3D", AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y)), 70.f, Rgba8::WHITE, 1.f, Vec2(0.5f, 0.7f));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->BindTexture(&m_font->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
}

void Game::RenderMatch() const
{
	m_theMatch->Render();
}

void Game::DrawBasis() const
{
	std::vector<Vertex_PCU> basisVerts;
	Vec3 arrowPosition = m_cameraPosition + Vec3::MakeFromPolarDegrees(m_cameraOrientation.m_pitchDegrees, m_cameraOrientation.m_yawDegrees) * 0.2f;
	float axisLength = 0.01f;
	float shaftFactor = 0.6f;
	float shaftRadius = 0.0008f;
	float coneRadius = 0.0015f;

	Vec3 xConeEnd = arrowPosition + Vec3::XAXE * axisLength;
	Vec3 yConeEnd = arrowPosition + Vec3::YAXE * axisLength;
	Vec3 zConeEnd = arrowPosition + Vec3::ZAXE * axisLength;

	Vec3 xCylinderEnd = arrowPosition + Vec3::XAXE * (axisLength * shaftFactor);
	Vec3 yCylinderEnd = arrowPosition + Vec3::YAXE * (axisLength * shaftFactor);
	Vec3 zCylinderEnd = arrowPosition + Vec3::ZAXE * (axisLength * shaftFactor);

	AddVertsForCylinder3D(basisVerts, arrowPosition, xCylinderEnd, shaftRadius, Rgba8::RED, AABB2::ZERO_TO_ONE, 32);
	AddVertsForCylinder3D(basisVerts, arrowPosition, yCylinderEnd, shaftRadius, Rgba8::GREEN, AABB2::ZERO_TO_ONE, 32);
	AddVertsForCylinder3D(basisVerts, arrowPosition, zCylinderEnd, shaftRadius, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 32);

	AddVertsForCone3D(basisVerts, xCylinderEnd, xConeEnd, coneRadius, Rgba8::RED, AABB2::ZERO_TO_ONE, 32);
	AddVertsForCone3D(basisVerts, yCylinderEnd, yConeEnd, coneRadius, Rgba8::GREEN, AABB2::ZERO_TO_ONE, 32);
	AddVertsForCone3D(basisVerts, zCylinderEnd, zConeEnd, coneRadius, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 32);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(basisVerts);
}

GameState Game::GetCurrentGameState() const
{
	return m_currentState;
}

void Game::EnterState(GameState state)
{
	ExitState(m_currentState);
	m_currentState = state;

	switch (state)
	{
		case GameState::NONE:
		{
			break;
		}
		case GameState::ATTRACT:
		{
			SetupAttractButtons();
			break;
		}
		case GameState::LOBBY:
		{
			break;
		}
		case GameState::GAME_PLAYING:
		{
			InitializeChessMatch();
			break;
		}
		case GameState::PLAYER_ONE_TURN:
		{
			break;
		}
		case GameState::PLAYER_TWO_TURN:
		{
			break;
		}
		case GameState::FINISHED_MATCH:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

void Game::ExitState(GameState state)
{
	g_theUISystem->Clear();

	switch (state)
	{
		case GameState::NONE:
		{
			break;
		}
		case GameState::ATTRACT:
		{
			break;
		}
		case GameState::LOBBY:
		{
			break;
		}
		case GameState::GAME_PLAYING:
		{
			DestroyMatch();
			break;
		}
		case GameState::PLAYER_ONE_TURN:
		{
			break;
		}
		case GameState::PLAYER_TWO_TURN:
		{
			break;
		}
		case GameState::FINISHED_MATCH:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

bool Game::Event_ChessServerInfo(EventArgs& args)
{
	std::string addressArgs = args.GetValue("ip", "");
	int portArgs = args.GetValue("port", -1);

	bool isConnected = g_theNetwork->IsConnected();

	if (isConnected)
	{
		if (!addressArgs.empty() || portArgs != -1)
		{
			g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Cannot change IP or Port while connected.");
		}
	}
	else
	{
		if (!addressArgs.empty())
		{
			g_theNetwork->GetNetworkConfig().m_address = addressArgs;
		}
		if (portArgs != -1)
		{
			g_theNetwork->GetNetworkConfig().m_port = Stringf("%d", portArgs);
		}
	}

	std::string ip = g_theNetwork->GetNetworkConfig().m_address;
	std::string port = g_theNetwork->GetNetworkConfig().m_port;
	std::string status = isConnected ? "Connected" : "Disconnected";
	std::string gameState = GetGameStateAsString(g_theGame->GetCurrentGameState());

	g_theDevConsole->AddLine(Rgba8::YELLOW, "========= Server Info =========");
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("IP Address       : %s", ip.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Port             : %s", port.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Connection Status: %s", status.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Game State       : %s", gameState.c_str()));
	g_theDevConsole->AddLine(Rgba8::YELLOW, "===============================");

	return true;
}

bool Game::Event_ChessListen(EventArgs& args)
{
	if (g_theNetwork->IsConnected())
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Cannot start listening while connected.");
		return false;
	}

	int portArgs = args.GetValue("port", -1);
	if (portArgs != -1)
	{
		g_theNetwork->GetNetworkConfig().m_port = Stringf("%d", portArgs);
	}

	g_theNetwork->StartServer();
	return true;
}

bool Game::Event_Chess_Connect(EventArgs& args)
{
	if (g_theNetwork->IsConnected())
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_MAJOR, "Already connected to a server.");
		return false;
	}

	std::string addressArgs = args.GetValue("ip", "");
	int portArgs = args.GetValue("port", -1);

	if (!addressArgs.empty())
	{
		g_theNetwork->GetNetworkConfig().m_address = addressArgs;
	}
	if (portArgs != -1)
	{
		g_theNetwork->GetNetworkConfig().m_port = Stringf("%d", portArgs);
	}

	g_theNetwork->StartClient();

	std::string ip = g_theNetwork->GetNetworkConfig().m_address;
	std::string port = g_theNetwork->GetNetworkConfig().m_port;

	g_theDevConsole->AddLine(Rgba8::YELLOW, "========= Connecting to Server =========");
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Target IP         : %s", ip.c_str()));
	g_theDevConsole->AddLine(Rgba8::CYAN, Stringf("Target Port       : %s", port.c_str()));
	g_theDevConsole->AddLine(Rgba8::YELLOW, "========================================");

	return true;
}