#pragma once
#include "Game/ChessObject.hpp"
#include "Game/GameCommon.h"
#include "Game/ChessPieceDefinition.hpp"
#include "Game/ChessPiece.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/IntVec2.h"
#include <vector>
// -----------------------------------------------------------------------------
class Shader;
class VertexBuffer;
class IndexBuffer;
class Texture;
class ChessMatch;
// -----------------------------------------------------------------------------
class ChessBoard : public ChessObject
{
public:
	ChessBoard(ChessMatch* owner);
	~ChessBoard();

	void IntializeChessPieces();
	void PopulateChessPiecesOnBoard();

	void Update(float deltaSeconds) override;
	void Render() const override;
	void DrawChessBoard() const;
	void RenderChessPieces() const;

	void RaycastVsBoardAndPieces(ChessRaycastResult& raycastResult);
	void RaycastVsMe(ChessRaycastResult& result) const override;
	IntVec2 GetBoardCoordsForWorldPos(Vec3 const& worldPos) const;

	// ChessBoard helpers to help with Event_ChessMove: ChessMove from=e2 to=e4
	IntVec2 GetCoordsForNotation(std::string const& blockNotation) const;
	std::string GetNotationForCoords(IntVec2 const& coords) const;
	bool IsOutOfBounds(IntVec2 const& coords) const;
	bool AreKingsApart(ChessPiece* movingKing, IntVec2 toCoords);

	// Getting a ChessPiece at specific coordinates
	ChessPiece* GetChessPieceForCoords(int x, int y) const;
	ChessPiece* GetChessPieceForCoords(IntVec2 const& coords) const;
	void        RemoveChessPieceAtCoords(IntVec2 const& coords);
	void		AddChessPiece(ChessPiece* piece);
	void		RemoveChessPiece(ChessPiece* piece);

	std::vector<ChessPiece*> m_chessPieces;
	ChessPiece* m_board[CHESS_BOARD_COLUMNS][CHESS_BOARD_ROWS] = {};
	IntVec2 m_enpassantTargetSquare = -IntVec2::ONE;
	Shader* m_diffuseShader = nullptr;
private:
	void CreateBuffers();
	void CreateBoardGeometry();

private:
	ChessMatch* m_chessMatch = nullptr;
	VertexBuffer* m_boardVertexBuffer = nullptr;
	IndexBuffer* m_boardIndexBuffer = nullptr;
	Texture* m_texture = nullptr;
	Texture* m_textureNorm = nullptr;
	Texture* m_textureSpec = nullptr;

	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
};