#pragma once
#include <string>
#include <vector>
// -----------------------------------------------------------------------------
class VertexBuffer;
class IndexBuffer;
// -----------------------------------------------------------------------------
enum class ChessPieceType
{
	CHESSPIECE_INVALID = -1,
	ROOK,
	KNIGHT,
	BISHOP,
	QUEEN,
	KING,
	PAWN,
	NUM_CHESSPIECETYPES
};
// -----------------------------------------------------------------------------
class ChessPieceDefinition 
{
public:
	static std::vector<ChessPieceDefinition> s_chessPieceDefs;

public:
	static void InitializeChessDefs();
	static ChessPieceDefinition const& GetChessPieceDef(ChessPieceType chessPieceType);
	static void SetChessPieceDefs(ChessPieceType type, const std::string& name, char glyph, VertexBuffer* whiteVBO, IndexBuffer* whiteIBO, VertexBuffer* blackVBO, IndexBuffer* blackIBO);
	void DrawProceduralMeshes(int playerIndex);

public:
	ChessPieceType m_chessPieceType = ChessPieceType::CHESSPIECE_INVALID;
	std::string m_chessPieceName = "default";
	char m_glyph = '-';

	// 2 VertexBuffers for white and black
	VertexBuffer* m_vertexBuffer[2] = { nullptr, nullptr };
	// 2 IndexBuffers default to nullptr
	IndexBuffer* m_indexBuffer[2] = { nullptr, nullptr };
};