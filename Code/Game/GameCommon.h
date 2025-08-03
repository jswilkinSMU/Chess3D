#pragma once
#include "Engine/Math/RandomNumberGenerator.h"
#include <string>
// -----------------------------------------------------------------------------
class App;
class Game;
class Renderer;
class InputSystem;
class AudioSystem;
class UISystem;
class NetworkSystem;
class Window;
struct Vec2;
struct Rgba8;
// -----------------------------------------------------------------------------
constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;
// -----------------------------------------------------------------------------
enum class GameState
{
	NONE,
	ATTRACT,
	LOBBY,
	GAME_PLAYING,
	PLAYER_ONE_TURN,
	PLAYER_TWO_TURN,
	FINISHED_MATCH,
	COUNT
};
// -----------------------------------------------------------------------------
enum class CameraState
{
	CAMERA_INVALID = -1,
	FREEFLY,
	PLAYERONE_SPECTATING,
	PLAYERTWO_SPECTATING,
	NUM_CAMERAS
};
// -----------------------------------------------------------------------------
enum class ChessMoveResult
{
	UNKNOWN = -1,
	VALID_MOVE_NORMAL,
	VALID_MOVE_PROMOTION,
	VALID_CASTLE_KINGSIDE,
	VALID_CASTLE_QUEENSIDE,
	VALID_CAPTURE_NORMAL,
	VALID_CAPTURE_ENPASSANT,
	INVALID_MOVE_BAD_LOCATION,
	INVALID_MOVE_NO_PIECE,
	INVALID_MOVE_NOT_YOUR_PIECE,
	INVALID_MOVE_ZERO_DISTANCE,
	INVALID_MOVE_WRONG_MOVE_SHAPE,
	INVALID_MOVE_DESTINATION_BLOCKED,
	INVALID_MOVE_PATH_BLOCKED,
	INVALID_MOVE_KINGS_APART,
	INVALID_MOVE_ENDS_IN_CHECK,
	INVALID_ENPASSANT_STALE,
	INVALID_CASTLE_KING_HAS_MOVED,
	INVALID_CASTLE_ROOK_HAS_MOVED,
	INVALID_CASTLE_PATH_BLOCKED,
	INVALID_CASTLE_THROUGH_CHECK,
	INVALID_CASTLE_OUT_OF_CHECK
};
bool IsChessMoveValid(ChessMoveResult chessMoveResult);
char const* GetChessMoveResultString(ChessMoveResult chessMoveResult);
// -----------------------------------------------------------------------------
constexpr int CHESS_BOARD_ROWS = 8;
constexpr int CHESS_BOARD_COLUMNS = 8;
constexpr int CHESS_BOARD_SIZE = CHESS_BOARD_ROWS * CHESS_BOARD_COLUMNS;
// -----------------------------------------------------------------------------
extern App* g_theApp;
extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern RandomNumberGenerator* g_rng;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern UISystem* g_theUISystem;
extern NetworkSystem* g_theNetwork;
extern Window* g_theWindow;
// -----------------------------------------------------------------------------
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
char const* GetDebugRenderModeDesc(int debugRenderMode);
std::string GetGameStateAsString(GameState gameState);