#include "Game/ChessPiece.hpp"
#include "Game/ChessBoard.hpp"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Vertex_PCU.h"
#include "Engine/Core/VertexUtils.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.cpp"
#include "Engine/Math/RaycastUtils.hpp"

ChessPiece::ChessPiece(ChessPieceDefinition const& chessPieceDef, int ownerIndex, ChessBoard* board)
	:m_chessPieceDef(&chessPieceDef),
	 m_ownerIndex(ownerIndex),
	 m_chessBoard(board)
{
	// Get wood textures and normal map
	//m_wood = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/240{A011}.jpg");
	//m_woodNorm = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/240_norm{A011}.jpg");

	// Get funky brick textures for player 1
	m_funkyBrick = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FunkyBricks_d.png");
	m_funkyBrickNorm = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FunkyBricks_n.png");
	m_funkyBrickSpec = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FunkyBricks_sge.png");

	// Get brick textures and normal map
	//m_brick = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/220{A011}.jpg");
	//m_brickNorm = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/220_norm{A011}.jpg");

	// Get cobblestone textures for player 2
	m_cobbleStone = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Cobblestone_Diffuse.png");
	m_cobbleStoneNorm = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Cobblestone_Normal.png");
	m_cobbleStoneSpec = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Cobblestone_SpecGlossEmit.png");

	// Create Chess Piece Geometry and buffers
	CreatePieceGeometry();
	CreateBuffers();
}

ChessPiece::~ChessPiece()
{
	delete m_vertexBuffer[0];
	m_vertexBuffer[0] = nullptr;
	delete m_indexBuffer[0];
	m_indexBuffer[0] = nullptr;

	delete m_vertexBuffer[1];
	m_vertexBuffer[1] = nullptr;
	delete m_indexBuffer[1];
	m_indexBuffer[1] = nullptr;
}

void ChessPiece::Update(float deltaseconds)
{
	Vec3 prevPos = m_position;
	Vec3 nextPos = m_targetPosition;

	float moveDistance = GetDistance3D(prevPos, nextPos);
	float moveDuration = 0.f;
	if (m_chessPieceDef->m_chessPieceType == ChessPieceType::KNIGHT)
	{
		moveDuration = (0.5f + (0.05f * moveDistance)) * 5.f;
	}
	else
	{
		moveDuration = (0.5f + (0.05f * moveDistance)) * 1000.f;
	}

	if (m_isMoving)
	{
		float t = GetClampedZeroToOne(m_secondsMoved / moveDuration);
		float smoothT = SmoothStop5(t);
		Vec3 lerpToNext = Interpolate(prevPos, nextPos, smoothT);

		float parabolicJump = 0.f;
		if (m_chessPieceDef->m_chessPieceType == ChessPieceType::KNIGHT)
		{
			float jumpHeight = 0.5f;
			parabolicJump = 4.f * jumpHeight * smoothT * (1.f - smoothT);
		}
		lerpToNext.z += parabolicJump;

		m_position = lerpToNext;
		m_secondsMoved += deltaseconds;

		if (m_secondsMoved >= moveDuration)
		{
			m_position = m_targetPosition;
			m_isMoving = false;
		}
	}
}

void ChessPiece::Render() const
{
	Vec3 position = m_position;
	Mat44 modelMatrix = Mat44::MakeTranslation3D(position);

	g_theRenderer->SetModelConstants(modelMatrix);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(m_chessBoard->m_diffuseShader);
	g_theRenderer->BindSampler(SamplerMode::POINT_CLAMP, 0); 
	g_theRenderer->BindSampler(SamplerMode::BILINEAR_WRAP, 1);
	g_theRenderer->BindSampler(SamplerMode::BILINEAR_WRAP, 2);
	if (m_ownerIndex == 0)
	{
		g_theRenderer->BindTexture(m_funkyBrick, 0);
		g_theRenderer->BindTexture(m_funkyBrickNorm, 1);
		g_theRenderer->BindTexture(m_funkyBrickSpec, 2);
		g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer[0], m_indexBuffer[0], static_cast<unsigned int>(m_indexes.size()));
	}
	else
	{
		g_theRenderer->BindTexture(m_cobbleStone, 0);
		g_theRenderer->BindTexture(m_cobbleStoneNorm, 1);
		g_theRenderer->BindTexture(m_cobbleStoneSpec, 2);
		g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer[1], m_indexBuffer[1], static_cast<unsigned int>(m_indexes.size()));
	}
}

void ChessPiece::RaycastVsMe(ChessRaycastResult& result) const
{
	Vec3 basePosition = Vec3::ZERO;
	float radius = 0, height = 0;
	GetCylinderForRaycastPiece(basePosition, radius, height);

	RaycastResult3D raycastResult = RaycastVsCylinder3D(result.m_rayStartPosition, result.m_rayFwdNormal, result.m_rayLength, basePosition, radius, height);

	if (raycastResult.m_didImpact)
	{
		if (!result.m_didImpact || raycastResult.m_impactDist < result.m_impactDist)
		{
			result.m_didImpact = true;
			result.m_coords = m_boardPosition;
			result.m_chessPiece = const_cast<ChessPiece*>(this);
			result.m_chessObject = const_cast<ChessPiece*>(this);
			result.m_impactPos = raycastResult.m_impactPos;
			result.m_impactNormal = raycastResult.m_impactNormal;
			result.m_impactDist = raycastResult.m_impactDist;
		}
	}
}

void ChessPiece::GetCylinderForRaycastDebug(Vec3& base, Vec3& top, float& radius) const
{
	base = GetWorldPosition();
	radius = 0.35f;

	switch (m_chessPieceDef->m_chessPieceType)
	{
		case ChessPieceType::PAWN:
			top = base + Vec3(0.f, 0.f, 0.6f);
			radius = 0.2f;
			break;
		case ChessPieceType::ROOK:
			top = base + Vec3(0.f, 0.f, 0.6f);
			break;
		case ChessPieceType::KNIGHT:
			top = base + Vec3(0.f, 0.f, 0.6f);
			break;
		case ChessPieceType::BISHOP:
			top = base + Vec3(0.f, 0.f, 0.8f);
			radius = 0.2f;
			break;
		case ChessPieceType::QUEEN:
			top = base + Vec3(0.f, 0.f, 0.9f);
			break;
		case ChessPieceType::KING:
			top = base + Vec3(0.f, 0.f, 1.15f);
			break;
		default:
			top = base + Vec3(0.f, 0.f, 1.0f);
			break;
	}
}

void ChessPiece::GetCylinderForRaycastPiece(Vec3& base, float& radius, float& height) const
{
	base = GetWorldPosition();
	radius = 0.35f;

	switch (m_chessPieceDef->m_chessPieceType)
	{
		case ChessPieceType::PAWN:
			height = 0.6f;
			radius = 0.2f;
			break;
		case ChessPieceType::ROOK:
			height = 0.6f;
			break;
		case ChessPieceType::KNIGHT:
			height = 0.6f;
			break;
		case ChessPieceType::BISHOP:
			height = 0.8f;
			radius = 0.2f;
			break;
		case ChessPieceType::QUEEN:
			height = 0.9f;
			break;
		case ChessPieceType::KING:
			height = 1.15f;
			break;
		default:
			height = 1.0f;
			break;
	}
}

void ChessPiece::SetBoardPosition(IntVec2 const& boardPos)
{
	m_prevBoardPosition = m_boardPosition;
	m_boardPosition = boardPos;
	m_targetPosition = Vec3(static_cast<float>(boardPos.x) + 0.5f, static_cast<float>(boardPos.y) + 0.5f, 0.f);

	m_chessBoard->m_board[boardPos.x][boardPos.y] = this;

	if (m_position == Vec3::ZERO)
	{
		m_position = m_targetPosition;
		m_secondsMoved = 0.f;
	}

	if (m_position != m_targetPosition)
	{
		m_isMoving = true;
		m_secondsMoved = 0.f;
	}
}

Vec3 ChessPiece::GetWorldPosition() const
{
	Vec3 boardPosAsWorldPos = Vec3(static_cast<float>(m_boardPosition.x) + 0.5f, static_cast<float>(m_boardPosition.y) + 0.5f, 0.f);
	return boardPosAsWorldPos;
}

void ChessPiece::CreatePieceGeometry()
{
	Rgba8 pieceColor = (m_ownerIndex == 0) ? Rgba8::GREEN : Rgba8::BROWN;

	if (m_chessPieceDef->m_chessPieceType == ChessPieceType::PAWN)
	{
		AddVertsForCylinderZ3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.2f), 0.2f, 0.4f, pieceColor, AABB2::ZERO_TO_ONE, 160);
		AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.5f), 0.2f, pieceColor, AABB2::ZERO_TO_ONE, 160, 152); 
	}
	else if (m_chessPieceDef->m_chessPieceType == ChessPieceType::ROOK)
	{
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.3f, -0.3f, 0.f), Vec3(0.3f, 0.3f, 0.1f)), pieceColor);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.15f, -0.15f, 0.1f), Vec3(0.15f, 0.15f, 0.5f)), pieceColor);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.3f, -0.3f, 0.5f), Vec3(0.3f, 0.3f, 0.6f)), pieceColor);
	}
	else if (m_chessPieceDef->m_chessPieceType == ChessPieceType::KNIGHT)
	{
		AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.f), 0.3f, pieceColor, AABB2::ZERO_TO_ONE, 160, 152);
		AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.4f), 0.2f, pieceColor, AABB2::ZERO_TO_ONE, 160, 152);
	}
	else if (m_chessPieceDef->m_chessPieceType == ChessPieceType::BISHOP)
	{
		AddVertsForCylinderZ3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.f), 0.2f, 0.5f, pieceColor, AABB2::ZERO_TO_ONE, 160);
		AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.55f), 0.15f, pieceColor, AABB2::ZERO_TO_ONE, 160, 152);
		AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.7f), 0.05f, pieceColor, AABB2::ZERO_TO_ONE, 160, 152);
	}
	else if (m_chessPieceDef->m_chessPieceType == ChessPieceType::QUEEN)
	{
		AddVertsForCylinderZ3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.f), 0.25f, 0.6f, pieceColor, AABB2::ZERO_TO_ONE, 160);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.25f, -0.25f, 0.6f), Vec3(0.25f, 0.25f, 0.75f)), pieceColor);

		float radius = 0.2f;
		int numSpheres = 8;
		float angleStep = 2 * PI / numSpheres;

		for (int sphereIndex = 0; sphereIndex < numSpheres; ++sphereIndex)
		{
			float angle = sphereIndex * angleStep;
			float x = radius * cos(angle);
			float y = radius * sin(angle);
			AddVertsForSphere3D(m_vertexes, m_indexes, Vec3(x, y, 0.75f), 0.05f, pieceColor);
		}
	}
	else if (m_chessPieceDef->m_chessPieceType == ChessPieceType::KING)
	{
		AddVertsForCylinderZ3D(m_vertexes, m_indexes, Vec3(0.f, 0.f, 0.f), 0.25f, 0.6f, pieceColor, AABB2::ZERO_TO_ONE, 160);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.25f, -0.25f, 0.6f), Vec3(0.25f, 0.25f, 0.75f)), pieceColor);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.05f, -0.05f, 0.75f), Vec3(0.05f, 0.05f, 0.85f)), pieceColor);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.05f, -0.05f, 0.85f), Vec3(0.05f, 0.05f, 1.15f)), pieceColor);
		AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(Vec3(-0.15f, -0.05f, 1.0f), Vec3(0.15f, 0.05f, 1.1f)), pieceColor);
	}
}

void ChessPiece::CreateBuffers()
{
	// Create buffers WHITE and copy to GPU
	m_vertexBuffer[0] = g_theRenderer->CreateVertexBuffer(static_cast<unsigned int>(m_vertexes.size()) * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer[0] = g_theRenderer->CreateIndexBuffer(static_cast<unsigned int>(m_indexes.size()) * sizeof(unsigned int), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_vertexBuffer[0]->GetSize(), m_vertexBuffer[0]);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexBuffer[0]->GetSize(), m_indexBuffer[0]);

	// Create buffers BLACK and copy to GPU
	m_vertexBuffer[1] = g_theRenderer->CreateVertexBuffer(static_cast<unsigned int>(m_vertexes.size()) * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer[1] = g_theRenderer->CreateIndexBuffer(static_cast<unsigned int>(m_indexes.size()) * sizeof(unsigned int), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_vertexBuffer[1]->GetSize(), m_vertexBuffer[1]);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexBuffer[1]->GetSize(), m_indexBuffer[1]);
}

ChessMoveResult ChessPiece::CheckMoveValidity(IntVec2 fromCoords, IntVec2 toCoords, std::string out_errorMessage)
{
	// Check for Out of bounds
	if (m_chessBoard->IsOutOfBounds(toCoords))
	{
		out_errorMessage = "Move is not on the board!";
		return ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
	}

	// Check for it move is the same spot
	if (fromCoords == toCoords)
	{
		out_errorMessage = "Move is in the same position, did not change!";
		return ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
	}

	ChessPiece* pieceAtFromCoords = m_chessBoard->GetChessPieceForCoords(fromCoords);
	ChessPiece* pieceAtToCoords = m_chessBoard->GetChessPieceForCoords(toCoords);

	// Check for if there is no piece at specified square
	if (!pieceAtFromCoords)
	{
		out_errorMessage = "No chess piece at stated square!";
		return ChessMoveResult::INVALID_MOVE_NO_PIECE;
	}

	// Check for wrong piece
	if (pieceAtFromCoords->GetPlayerIndex() != m_ownerIndex) 
	{
		out_errorMessage = "You're trying to move the opponent's piece!";
		return ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
	}

	// Helper bools for captured and blocked
	bool isCaptured = pieceAtToCoords && pieceAtToCoords->GetPlayerIndex() != m_ownerIndex;
	bool isBlockedByOwnPiece = pieceAtToCoords && pieceAtToCoords->GetPlayerIndex() == m_ownerIndex;
	bool isFirstMove = (m_prevBoardPosition == IntVec2::ZERO && m_boardPosition == fromCoords);

	// Check if we are blocked by our own chess piece
	if (isBlockedByOwnPiece)
	{
		out_errorMessage = "Destination is occupied by your own piece!";
		return ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
	}

	switch (m_chessPieceDef->m_chessPieceType)
	{
		case ChessPieceType::PAWN:	 return ValidatePawnMove(fromCoords, toCoords, isCaptured, isFirstMove, out_errorMessage);
		case ChessPieceType::KNIGHT: return ValidateKnightMove(fromCoords, toCoords, isCaptured, out_errorMessage);
		case ChessPieceType::BISHOP: return ValidateBishopMove(fromCoords, toCoords, isCaptured, out_errorMessage);
		case ChessPieceType::ROOK:   return ValidateRookMove(fromCoords, toCoords, isCaptured, out_errorMessage);
		case ChessPieceType::QUEEN:  return ValidateQueenMove(fromCoords, toCoords, isCaptured, out_errorMessage);
		case ChessPieceType::KING:   return ValidateKingMove(fromCoords, toCoords, isCaptured, out_errorMessage);
		default:
		{
			out_errorMessage = "Unknown piece type.";
			return ChessMoveResult::UNKNOWN;
		}
	}
}

ChessMoveResult ChessPiece::ValidatePawnMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, bool isFirstMove, std::string& out_errorMessage)
{
	int direction = (m_ownerIndex == 0) ? 1 : -1;
	IntVec2 squareMoved = toCoords - fromCoords;

	if (squareMoved.x == 0 && squareMoved.y == direction && !m_chessBoard->GetChessPieceForCoords(toCoords)) 
	{
		if (CanPromote(toCoords.y))
		{
			return ChessMoveResult::VALID_MOVE_PROMOTION;
		}
		else
		{
			return ChessMoveResult::VALID_MOVE_NORMAL;
		}
	}

	if (squareMoved.x == 0 && squareMoved.y == 2 * direction && isFirstMove) 
	{
		IntVec2 middle = fromCoords + IntVec2(0, direction);
		if (!m_chessBoard->GetChessPieceForCoords(middle) && !m_chessBoard->GetChessPieceForCoords(toCoords)) 
		{
			m_chessBoard->m_enpassantTargetSquare = middle;
			return ChessMoveResult::VALID_MOVE_NORMAL;
		}
		else 
		{
			out_errorMessage = "Path blocked on pawn double move.";
			return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
		}
	}

	// Check for enpassant capturing with pawn
	if (squareMoved.x == 1 && squareMoved.y == direction && !isCaptured)
	{
		ChessPiece* enemyPawn = m_chessBoard->GetChessPieceForCoords(IntVec2(fromCoords.x + squareMoved.x, fromCoords.y));
		if (enemyPawn != nullptr && enemyPawn->GetPlayerIndex() != m_ownerIndex && enemyPawn->GetDefinition()->m_chessPieceType == ChessPieceType::PAWN)
		{
			if (m_chessBoard->m_enpassantTargetSquare == toCoords)
			{
				m_chessBoard->RemoveChessPiece(enemyPawn);
				return ChessMoveResult::VALID_CAPTURE_ENPASSANT;
			}
			else
			{
				out_errorMessage = "EnPassant is invalid, enemy pawn did not move two squares";
				return ChessMoveResult::INVALID_ENPASSANT_STALE;
			}
		}
	}
	else if (squareMoved.x == -1 && squareMoved.y == direction && !isCaptured)
	{
		ChessPiece* enemyPawn = m_chessBoard->GetChessPieceForCoords(IntVec2(fromCoords.x + squareMoved.x, fromCoords.y));
		if (enemyPawn != nullptr && enemyPawn->GetPlayerIndex() != m_ownerIndex && enemyPawn->GetDefinition()->m_chessPieceType == ChessPieceType::PAWN)
		{
			if (m_chessBoard->m_enpassantTargetSquare == toCoords)
			{
				m_chessBoard->RemoveChessPiece(enemyPawn);
				return ChessMoveResult::VALID_CAPTURE_ENPASSANT;
			}
			else
			{
				out_errorMessage = "EnPassant is invalid, enemy pawn did not move two squares";
				return ChessMoveResult::INVALID_ENPASSANT_STALE;
			}
		}
	}

	// Check for regular capturing with pawn
	if (abs(squareMoved.x) == 1 && squareMoved.y == direction && isCaptured)
	{
		return ChessMoveResult::VALID_CAPTURE_NORMAL;
	}

	out_errorMessage = "Invalid pawn move shape.";
	return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
}

ChessMoveResult ChessPiece::ValidateKnightMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage)
{
	int mDist = GetTaxicabDistance2D(fromCoords, toCoords);
	int dx = abs(toCoords.x - fromCoords.x);
	int dy = abs(toCoords.y - fromCoords.y);

	bool xMove = (dx == 2 && dy == 1);
	bool yMove = (dy == 2 && dx == 1);

	if (mDist == 3 && (xMove || yMove))
	{
		if (isCaptured)
		{
			return ChessMoveResult::VALID_CAPTURE_NORMAL;
		}
		else
		{
			return ChessMoveResult::VALID_MOVE_NORMAL;
		}
	}

	out_errorMessage = "Invalid knight move: Must move in L shape";
	return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
}

ChessMoveResult ChessPiece::ValidateBishopMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage)
{
	int dx = abs(toCoords.x - fromCoords.x);
	int dy = abs(toCoords.y - fromCoords.y);

	// Check if we are moving diagonally across the board
	if (dx != dy)
	{
		out_errorMessage = "Invalid bishop move: Must move diagonally";
		return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
	}

	// Check if our bishop's path is blocked
	if (!IsPathOpen(fromCoords, toCoords))
	{
		out_errorMessage = "Bishops path is blocked by another piece";
		return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
	}

	if (isCaptured)
	{
		return ChessMoveResult::VALID_CAPTURE_NORMAL;
	}
	else
	{
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
}

ChessMoveResult ChessPiece::ValidateRookMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage)
{
	int dx = abs(toCoords.x - fromCoords.x);
	int dy = abs(toCoords.y - fromCoords.y);

	if (!(dx == 0 || dy == 0))
	{
		out_errorMessage = "Invalid Rook move: Must move in a horizontal or vertical line";
		return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
	}

	if (!IsPathOpen(fromCoords, toCoords))
	{
		out_errorMessage = "Rooks path is blocked by another piece";
		return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
	}

	if (isCaptured)
	{
		return ChessMoveResult::VALID_CAPTURE_NORMAL;
	}
	else
	{
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
}

ChessMoveResult ChessPiece::ValidateQueenMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage)
{
	int dx = abs(toCoords.x - fromCoords.x);
	int dy = abs(toCoords.y - fromCoords.y);
	bool diagonalMove = (dx == dy);
	bool straightMove = (dx == 0 || dy == 0);

	if (!diagonalMove && !straightMove)
	{
		out_errorMessage = "Invalid Queen move: Must move queen in a straight or diagonal direction";
		return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
	}

	if (!IsPathOpen(fromCoords, toCoords))
	{
		out_errorMessage = "Queens path is blocked by another piece";
		return ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
	}

	if (isCaptured)
	{
		return ChessMoveResult::VALID_CAPTURE_NORMAL;
	}
	else
	{
		return ChessMoveResult::VALID_MOVE_NORMAL;
	}
}

ChessMoveResult ChessPiece::ValidateCastlingMove(IntVec2 fromCoords, IntVec2 toCoords, std::string& out_errorMessage)
{
	bool isKingside = toCoords.x > fromCoords.x;
	IntVec2 rookPos = isKingside ? IntVec2(7, fromCoords.y) : IntVec2(0, fromCoords.y);
	ChessPiece* rook = m_chessBoard->GetChessPieceForCoords(rookPos);

	if (!rook || rook->GetDefinition()->m_chessPieceType != ChessPieceType::ROOK || rook->GetPlayerIndex() != m_ownerIndex)
	{
		out_errorMessage = "Castling rook not found";
		return ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
	}

	// Check that king hasn't moved
	if (this->HasMoved())
	{
		out_errorMessage = "You can't castle: King has moved!";
		return ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
	}
	// Check that rook hasn't moved
	if (rook->HasMoved())
	{
		out_errorMessage = "You can't castle: Rook has moved!";
		return ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
	}

	// Check path is clear
	int minX = (fromCoords.x < rookPos.x) ? fromCoords.x : rookPos.x;
	int maxX = (fromCoords.x > rookPos.x) ? fromCoords.x : rookPos.x;
	for (int x = minX + 1; x < maxX; ++x)
	{
		if (m_chessBoard->GetChessPieceForCoords(IntVec2(x, fromCoords.y)))
		{
			out_errorMessage = "Castling path is blocked!";
			return ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
		}
	}

	if (isKingside)
	{
		return ChessMoveResult::VALID_CASTLE_KINGSIDE;
	}
	else
	{
		return ChessMoveResult::VALID_CASTLE_QUEENSIDE;
	}
}

ChessMoveResult ChessPiece::ValidateKingMove(IntVec2 fromCoords, IntVec2 toCoords, bool isCaptured, std::string& out_errorMessage)
{
	int mDist = GetTaxicabDistance2D(fromCoords, toCoords);
	int dx = abs(toCoords.x - fromCoords.x);
	int dy = abs(toCoords.y - fromCoords.y);

	if (dy == 0 && dx == 2)
	{
		return ValidateCastlingMove(fromCoords, toCoords, out_errorMessage);
	}

	if (mDist >= 1 && mDist <= 2 && dx <= 1 && dy <= 1)
	{
		if (!m_chessBoard->AreKingsApart(this, toCoords))
		{
			out_errorMessage = "King cannot be adjacent to opponents king!";
			return ChessMoveResult::INVALID_MOVE_KINGS_APART;
		}

		if (isCaptured)
		{
			return ChessMoveResult::VALID_CAPTURE_NORMAL;
		}
		else
		{
			return ChessMoveResult::VALID_MOVE_NORMAL;
		}
	}

	out_errorMessage = "Invalid king move: Kings can only move one square in any direction";
	return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
}


bool ChessPiece::CanPromote(int rank)
{
	return (m_ownerIndex == 0 && rank == 7) || (m_ownerIndex == 1 && rank == 0);
}

bool ChessPiece::IsPathOpen(IntVec2 fromCoords, IntVec2 toCoords)
{
	IntVec2 squareMoved = toCoords - fromCoords;
	int dx = (squareMoved.x == 0) ? 0 : (squareMoved.x > 0 ? 1 : -1);
	int dy = (squareMoved.y == 0) ? 0 : (squareMoved.y > 0 ? 1 : -1);

	IntVec2 pathDir = IntVec2(dx, dy);
	for (IntVec2 pos = fromCoords + pathDir; pos != toCoords; pos += pathDir)
	{
		if (m_chessBoard->GetChessPieceForCoords(pos) != nullptr)
		{
			return false;
		}
	}
	return true;
}