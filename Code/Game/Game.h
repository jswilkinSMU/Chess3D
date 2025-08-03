#pragma once
#include "Game/GameCommon.h"
#include "Game/ChessObject.hpp"
#include "Engine/Renderer/Camera.h"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Vertex_PCU.h"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
// -----------------------------------------------------------------------------
class ChessMatch;
class ChessPiece;
class BitmapFont;
// -----------------------------------------------------------------------------
class Game
{
public:
	App* m_app;
	Game(App* owner);
	~Game();

	// Initialization
	void StartUp();
	void InitializeChessMatch();
	void SetupAttractButtons();

	// Updating
	void Update();
	void UpdateUIPresses(float deltaSeconds);
	void DebugVisuals();

	// Raycasting
	void UpdateRaycast();
	void UpdateHighlighted();
	void UpdateSelected();
	ChessPiece* GetSelectedPiece() const;

	// Camera
	void UpdateCameras(float deltaSeconds);
	void CameraSwitch();
	void FreeFlyControls(float deltaSeconds);
	Vec3 GetCameraFwdNormal() const;

	// Rendering
	void Render() const;
	void RenderAttractMode() const;
	void RenderMatch() const;
	void DrawBasis() const;

	// Game state
	GameState GetCurrentGameState() const;
	void EnterState(GameState state);
	void ExitState(GameState state);

	// Events
	static bool Event_ChessServerInfo(EventArgs& args);
	static bool Event_ChessListen(EventArgs& args);
	static bool Event_Chess_Connect(EventArgs& args);

	// Shutting down
	void Shutdown();
	void DestroyMatch();

	// Key presses
	void KeyInputPresses();
	void AdjustForPauseAndTimeDistortion(float deltaSeconds);

public:
	GameState	m_currentState = GameState::ATTRACT;
	CameraState m_currentCameraState = CameraState::FREEFLY;
	ChessMatch* m_theMatch = nullptr;
	int			m_debugInt = 0;

	SoundID m_errorSound;
	SoundID m_chessSlideSound;

private:
	Camera		m_screenCamera;
	Camera      m_gameWorldCamera;
	Clock		m_gameClock;

	EulerAngles m_cameraOrientation = EulerAngles::ZERO;
	Vec3 m_cameraPosition = Vec3::ZERO;

	ChessRaycastResult m_lastRaycastResult;
	ChessPiece* m_selectedPiece = nullptr;
	IntVec2		m_highlightedSquare = -IntVec2::ONE;
	IntVec2		m_selectedCoords = -IntVec2::ONE;

	bool m_isRaycastDebug = false;
	BitmapFont* m_font = nullptr;

	// Sound
	std::string m_gameMusicPath;
	std::string m_clickSoundPath;
	SoundID     m_gameMusic;
	SoundID     m_clickSound;
	SoundID		m_clickOffSound;
	SoundID		m_gameStartSound;
	SoundPlaybackID m_gameMusicPlayback;
	float m_musicVolume = 0.0f;
};