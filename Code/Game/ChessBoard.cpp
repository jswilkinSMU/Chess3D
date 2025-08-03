#include "Game/ChessBoard.hpp"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/RaycastUtils.hpp"

/* GameCommon constexprs:
* CHESS_BOARD_ROWS
* CHESS_BOARD_COLUMNS
* CHESS_BOARD_SIZE
*/

ChessBoard::ChessBoard(ChessMatch* owner)
	:ChessObject()
{
	m_chessMatch = owner;
	m_diffuseShader = g_theRenderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::VERTEX_PCUTBN);
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Bricks_d.png");
	m_textureNorm = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Bricks_n.png");
	m_textureSpec = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Bricks_sge.png");

	IntializeChessPieces();
	CreateBoardGeometry();
	CreateBuffers();
	PopulateChessPiecesOnBoard();
}


ChessBoard::~ChessBoard()
{
	// ChessBoard will delete all 32 chess pieces once the match is destroyed
	for (int chessPieceIndex = 0; chessPieceIndex < static_cast<int>(m_chessPieces.size()); ++chessPieceIndex)
	{
		ChessPiece*& chessPiece = m_chessPieces[chessPieceIndex];
		if (chessPiece != nullptr)
		{
			delete chessPiece;
			chessPiece = nullptr;
		}
	}

	delete m_boardVertexBuffer;
	m_boardVertexBuffer = nullptr;
	
	delete m_boardIndexBuffer;
	m_boardIndexBuffer = nullptr;
}

void ChessBoard::IntializeChessPieces()
{
	// White pawns
	for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column)
	{
		m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::PAWN), 0, this));
	}

	// Black pawns
	for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column)
	{
		m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::PAWN), 1, this));
	}

	// Player 1
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::ROOK), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KNIGHT), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::BISHOP), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::QUEEN), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KING), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::BISHOP), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KNIGHT), 0, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::ROOK), 0, this));

	// Player 2
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::ROOK), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KNIGHT), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::BISHOP), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::QUEEN), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KING), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::BISHOP), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::KNIGHT), 1, this));
	m_chessPieces.push_back(new ChessPiece(ChessPieceDefinition::GetChessPieceDef(ChessPieceType::ROOK), 1, this));
}

void ChessBoard::PopulateChessPiecesOnBoard()
{
	for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column)
	{
		ChessPiece* whitePawn = m_chessPieces[column];
		whitePawn->SetBoardPosition(IntVec2(column, 1));
	}

	for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column)
	{
		ChessPiece* blackPawn = m_chessPieces[column + CHESS_BOARD_COLUMNS];
		blackPawn->SetBoardPosition(IntVec2(column, 6));
	}

	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 0]->SetBoardPosition(IntVec2(0, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 1]->SetBoardPosition(IntVec2(1, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 2]->SetBoardPosition(IntVec2(2, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 3]->SetBoardPosition(IntVec2(3, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 4]->SetBoardPosition(IntVec2(4, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 5]->SetBoardPosition(IntVec2(5, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 6]->SetBoardPosition(IntVec2(6, 0));
	m_chessPieces[CHESS_BOARD_COLUMNS * 2 + 7]->SetBoardPosition(IntVec2(7, 0));

	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 0]->SetBoardPosition(IntVec2(0, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 1]->SetBoardPosition(IntVec2(1, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 2]->SetBoardPosition(IntVec2(2, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 3]->SetBoardPosition(IntVec2(3, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 4]->SetBoardPosition(IntVec2(4, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 5]->SetBoardPosition(IntVec2(5, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 6]->SetBoardPosition(IntVec2(6, 7));
	m_chessPieces[CHESS_BOARD_COLUMNS * 3 + 7]->SetBoardPosition(IntVec2(7, 7));
}

void ChessBoard::Update(float deltaSeconds)
{
	for (int chessPieceIndex = 0; chessPieceIndex < static_cast<int>(m_chessPieces.size()); ++chessPieceIndex)
	{
		ChessPiece* chessPiece = m_chessPieces[chessPieceIndex];
		chessPiece->Update(deltaSeconds);
	}
}

void ChessBoard::Render() const
{
	DrawChessBoard();
	for (int chessPieceIndex = 0; chessPieceIndex < static_cast<int>(m_chessPieces.size()); ++chessPieceIndex)
	{
		ChessPiece* chessPiece = m_chessPieces[chessPieceIndex];
		chessPiece->Render();
	}
}

void ChessBoard::DrawChessBoard() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform());
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(m_diffuseShader);
	g_theRenderer->BindTexture(m_texture, 0);
	g_theRenderer->BindTexture(m_textureNorm, 1);
	g_theRenderer->BindTexture(m_textureSpec, 2);
	g_theRenderer->BindSampler(SamplerMode::POINT_CLAMP, 0);
	g_theRenderer->BindSampler(SamplerMode::BILINEAR_WRAP, 1);
	g_theRenderer->BindSampler(SamplerMode::BILINEAR_WRAP, 2);
	g_theRenderer->DrawIndexedVertexBuffer(m_boardVertexBuffer, m_boardIndexBuffer, static_cast<unsigned int>(m_indexes.size()));
}

void ChessBoard::RenderChessPieces() const
{
	for (int chessPieceIndex = 0; chessPieceIndex < static_cast<int>(m_chessPieces.size()); ++chessPieceIndex)
	{
		ChessPiece* chessPiece = m_chessPieces[chessPieceIndex];
		Vec3 position = chessPiece->GetWorldPosition();
		Mat44 modelMatrix = Mat44::MakeTranslation3D(position);
	}
}

void ChessBoard::RaycastVsBoardAndPieces(ChessRaycastResult& raycastResult)
{
	// First check pieces
	for (int chessPieceIndex = 0; chessPieceIndex < static_cast<int>(m_chessPieces.size()); ++chessPieceIndex)
	{
		ChessPiece* chessPiece = m_chessPieces[chessPieceIndex];
		chessPiece->RaycastVsMe(raycastResult);
	}

	// Then check board
	RaycastVsMe(raycastResult);
}

void ChessBoard::RaycastVsMe(ChessRaycastResult& result) const
{
	float squareSize = 1.f;

	for (int row = 0; row < CHESS_BOARD_ROWS; ++row)
	{
		for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column)
		{
			Vec3 mins = Vec3(column * squareSize, row * squareSize, -0.33f);
			Vec3 maxs = Vec3((column + 1) * squareSize, (row + 1) * squareSize, 0.f);
			AABB3 squareBounds(mins, maxs);

			RaycastResult3D squareHit = RaycastVsAABB3D(result.m_rayStartPosition, result.m_rayFwdNormal, result.m_rayLength, squareBounds);

			if (squareHit.m_didImpact)
			{
				if (!result.m_didImpact || squareHit.m_impactDist < result.m_impactDist)
				{
					result.m_coords = IntVec2(column, row);
					result.m_chessObject = const_cast<ChessBoard*>(this);
					result.m_didImpact = true;
					result.m_impactPos = squareHit.m_impactPos;
					result.m_impactNormal = squareHit.m_impactNormal;
					result.m_impactDist = squareHit.m_impactDist;
				}
			}
		}
	}
}

IntVec2 ChessBoard::GetBoardCoordsForWorldPos(Vec3 const& worldPos) const
{
	int x = RoundDownToInt(worldPos.x);
	int y = RoundDownToInt(worldPos.y);

	if (IsOutOfBounds(IntVec2(x, y)))
	{
		return -IntVec2::ONE;
	}

	return IntVec2(x, y);
}

IntVec2 ChessBoard::GetCoordsForNotation(std::string const& blockNotation) const
{
	if (blockNotation.size() != 2)
	{
		return -IntVec2::ONE;
	}

	char columnChar = blockNotation[0];
	char rowChar = blockNotation[1];

	if (columnChar < 'a' || columnChar > 'h' || rowChar < '1' || rowChar > '8')
	{
		return -IntVec2::ONE;
	}

	int columnX = columnChar - 'a';
	int rowY = (rowChar - '0') - 1;
	return IntVec2(columnX, rowY);
}

std::string ChessBoard::GetNotationForCoords(IntVec2 const& coords) const
{
	if (IsOutOfBounds(coords))
	{
		return "INVALID";
	}

	char columnChar = static_cast<char>('a' + coords.x);
	char rowChar = static_cast<char>('1' + coords.y);
	return std::string(1, columnChar) + std::string(1, rowChar);
}

bool ChessBoard::IsOutOfBounds(IntVec2 const& coords) const
{
	return (coords.x < 0 || coords.x >= CHESS_BOARD_COLUMNS) || 
		   (coords.y < 0 || coords.y >= CHESS_BOARD_ROWS);
}

bool ChessBoard::AreKingsApart(ChessPiece* movingKing, IntVec2 toCoords)
{
	ChessPiece* opponentKing = nullptr;
	for (int boardX = 0; boardX < CHESS_BOARD_COLUMNS; ++boardX)
	{
		for (int boardY = 0; boardY < CHESS_BOARD_ROWS; ++boardY)
		{
			ChessPiece* piece = m_board[boardX][boardY];
			if (piece != nullptr && piece->m_chessPieceDef->m_chessPieceType == ChessPieceType::KING && piece != movingKing)
			{
				opponentKing = piece;
				break;
			}
		}
	}

	if (opponentKing == nullptr)
	{
		return true;
	}

	int dx = abs(toCoords.x - opponentKing->GetBoardPosition().x);
	int dy = abs(toCoords.y - opponentKing->GetBoardPosition().y);
	return dx > 1 || dy > 1;
}

ChessPiece* ChessBoard::GetChessPieceForCoords(int x, int y) const
{
	if (IsOutOfBounds(IntVec2(x, y)))
	{
		return nullptr;
	}
	return m_board[x][y];
}

ChessPiece* ChessBoard::GetChessPieceForCoords(IntVec2 const& coords) const
{
	return GetChessPieceForCoords(coords.x, coords.y);
}

void ChessBoard::RemoveChessPieceAtCoords(IntVec2 const& coords)
{
	if (!IsOutOfBounds(coords)) 
	{
		m_board[coords.x][coords.y] = nullptr;
	}
}

void ChessBoard::AddChessPiece(ChessPiece* piece)
{
	if (piece == nullptr)
	{
		return;
	}
	m_chessPieces.push_back(piece);
}

void ChessBoard::RemoveChessPiece(ChessPiece* piece)
{
	IntVec2 coords = piece->GetBoardPosition();
	if (!IsOutOfBounds(coords))
	{
		m_board[coords.x][coords.y] = nullptr;
	}

	auto found = std::find(m_chessPieces.begin(), m_chessPieces.end(), piece);
	if (found != m_chessPieces.end())
	{
		m_chessPieces.erase(found);
	}
	delete piece;
}

void ChessBoard::CreateBuffers()
{
	if (m_vertexes.empty()) 
	{
		return;
	}
	if (m_indexes.empty()) 
	{
		return;
	}

	m_boardVertexBuffer = g_theRenderer->CreateVertexBuffer(static_cast<unsigned int>(m_vertexes.size()) * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_boardIndexBuffer = g_theRenderer->CreateIndexBuffer(static_cast<unsigned int>(m_indexes.size()) * sizeof(unsigned int), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_boardVertexBuffer->GetSize(), m_boardVertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_boardIndexBuffer->GetSize(), m_boardIndexBuffer);
}

void ChessBoard::CreateBoardGeometry()
{
	float squareSize = 1.0f;

	for (int row = 0; row < CHESS_BOARD_ROWS; ++row) 
	{
		for (int column = 0; column < CHESS_BOARD_COLUMNS; ++column) 
		{
			Vec3 boardMins = Vec3(column * squareSize, row * squareSize, -0.33f);
			Vec3 boardMaxs = Vec3((column + 1) * squareSize, (row + 1) * squareSize, 0.f);

			Rgba8 squarecolor = Rgba8::WHITE;
			if ((row + column) % 2 == 0)
			{
				squarecolor = Rgba8(10, 10, 10, 255);
			}
			else
			{
				squarecolor = Rgba8::WHITE;
			}

			AABB2 UVs(Vec2(static_cast<float>(column) / 8.0f, static_cast<float>(row) / 8.0f), Vec2(static_cast<float>(column + 1) / 8.0f, static_cast<float>(row + 1) / 8.0f));
			AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(boardMins, boardMaxs), squarecolor, AABB2::ZERO_TO_ONE);
		}
	}
}
