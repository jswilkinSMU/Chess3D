#pragma once
#include "Game/ChessObject.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Game/GameCommon.h"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/IntVec2.h"
#include <vector>
// -----------------------------------------------------------------------------
class Texture;
class ChessBoard;
// -----------------------------------------------------------------------------
class ChessPiece : public ChessObject
{
public:
	ChessPiece(ChessPieceDefinition const& chessPieceDef, int ownerIndex, ChessBoard* board);
	~ChessPiece();

	void Update(float deltaSeconds) override;
	void Render() const override;

	void RaycastVsMe(ChessRaycastResult& result) const override;
	void GetCylinderForRaycastDebug(Vec3& base, Vec3& top, float& radius) const;
	void GetCylinderForRaycastPiece(Vec3& base, float& radius, float& height) const;

	ChessPieceDefinition const* GetDefinition() { return m_chessPieceDef; }
	int GetPlayerIndex() const { return m_ownerIndex; }
	void SetBoardPosition(IntVec2 const& boardPos);
	Vec3    GetWorldPosition() const;
	IntVec2 GetBoardPosition() const { return m_boardPosition;  }
	bool	HasMoved() const { return m_prevBoardPosition != IntVec2::ZERO; }

	void CreatePieceGeometry();
	void CreateBuffers();

	// ChessPiece Move Validation functions
	ChessMoveResult CheckMoveValidity(IntVec2 fromCoords, IntVec2 toCoords, std::string out_errorMessage);
	ChessMoveResult ValidatePawnMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, bool isFirstMove, std::string& out_errorMessage);
	ChessMoveResult ValidateKnightMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage);
	ChessMoveResult ValidateBishopMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage);
	ChessMoveResult ValidateRookMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage);
	ChessMoveResult ValidateKingMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage);
	ChessMoveResult ValidateQueenMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage);
	ChessMoveResult ValidateCastlingMove(IntVec2 fromCoords, IntVec2 toCoords, std::string& out_errorMessage);
	bool CanPromote(int rank);
	bool IsPathOpen(IntVec2 fromCoords, IntVec2 toCoords);

public:
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	ChessPieceDefinition const* m_chessPieceDef = nullptr;
	int m_ownerIndex = 0;
	char m_chessPieceGlyph = '-';
	ChessBoard* m_chessBoard = nullptr;
	IntVec2 m_boardPosition = IntVec2::ZERO;
	IntVec2 m_prevBoardPosition = IntVec2::ZERO;
	float m_secondsMoved = 0.0f;
	Vec3 m_targetPosition = Vec3::ZERO;
	bool m_isMoving = false;

	// 2 VertexBuffers for white and black
	VertexBuffer* m_vertexBuffer[2] = { nullptr, nullptr };
	// 2 IndexBuffers default to nullptr
	IndexBuffer* m_indexBuffer[2] = { nullptr, nullptr };
	Texture* m_funkyBrick = nullptr;
	Texture* m_funkyBrickNorm = nullptr;
	Texture* m_funkyBrickSpec = nullptr;
	Texture* m_cobbleStone = nullptr;
	Texture* m_cobbleStoneNorm = nullptr;
	Texture* m_cobbleStoneSpec = nullptr;
};