#include "Game/ChessPieceDefinition.hpp"
#include "Game/GameCommon.h"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.h"
#include "Engine/Renderer/Renderer.h"

// -----------------------------------------------------------------------------
std::vector<ChessPieceDefinition> ChessPieceDefinition::s_chessPieceDefs;
// -----------------------------------------------------------------------------

void ChessPieceDefinition::InitializeChessDefs()
{
	// TODO: Refactor initialization to take the vertex and index buffers correctly
	s_chessPieceDefs.resize(static_cast<int>(ChessPieceType::NUM_CHESSPIECETYPES));
	SetChessPieceDefs(ChessPieceType::ROOK, "Rook", 'R', nullptr, nullptr, nullptr, nullptr);
	SetChessPieceDefs(ChessPieceType::KNIGHT, "Knight", 'N', nullptr, nullptr, nullptr, nullptr);
	SetChessPieceDefs(ChessPieceType::BISHOP, "Bishop", 'B', nullptr, nullptr, nullptr, nullptr);
	SetChessPieceDefs(ChessPieceType::QUEEN, "Queen", 'Q', nullptr, nullptr, nullptr, nullptr);
	SetChessPieceDefs(ChessPieceType::KING, "King", 'K', nullptr, nullptr, nullptr, nullptr);
	SetChessPieceDefs(ChessPieceType::PAWN, "Pawn", 'P', nullptr, nullptr, nullptr, nullptr);
}

ChessPieceDefinition const& ChessPieceDefinition::GetChessPieceDef(ChessPieceType chessPieceType)
{
	return s_chessPieceDefs[static_cast<int>(chessPieceType)];
}

void ChessPieceDefinition::SetChessPieceDefs(ChessPieceType chessPieceType, const std::string& name, char glyph, VertexBuffer* whiteVBO, IndexBuffer* whiteIBO, VertexBuffer* blackVBO, IndexBuffer* blackIBO)
{
	// TODO: Refactor initialization to take the vertex and index buffers correctly
	int index = static_cast<int>(chessPieceType);
	ChessPieceDefinition& def = s_chessPieceDefs[index];
	def.m_chessPieceType = chessPieceType;
	def.m_chessPieceName = name;
	def.m_glyph = glyph;
	def.m_vertexBuffer[0] = whiteVBO;
	def.m_indexBuffer[0] = whiteIBO;
	def.m_vertexBuffer[1] = blackVBO;
	def.m_indexBuffer[1] = blackIBO;
}

void ChessPieceDefinition::DrawProceduralMeshes(int playerIndex)
{
	std::vector<Vertex_PCUTBN> meshVerts;
	std::vector<unsigned int> meshIndices;
	meshVerts.reserve(10000);

	if (m_chessPieceType == ChessPieceType::PAWN)
	{
		// Draw pawn, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}
	else if (m_chessPieceType == ChessPieceType::ROOK)
	{
		// Draw rook, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}
	else if (m_chessPieceType == ChessPieceType::KNIGHT)
	{
		// Draw knight, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}
	else if (m_chessPieceType == ChessPieceType::BISHOP)
	{
		// Draw bishop, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}
	else if (m_chessPieceType == ChessPieceType::QUEEN)
	{
		// Draw queen, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}
	else if (m_chessPieceType == ChessPieceType::KING)
	{
		// Draw king, temporarily as sphere
		AddVertsForSphere3D(meshVerts, meshIndices, Vec3::ZERO, 2.f);
	}

	if (playerIndex == 0)
	{
		// Do something to differentiate the sides
		// Player 1 is White
	}
	else if (playerIndex == 1)
	{
		// Do something to differentiate the sides
		// Player 2 is Black
	}

	// Create buffers WHITE and copy to GPU
	m_vertexBuffer[0] = g_theRenderer->CreateVertexBuffer(static_cast<unsigned int>(meshVerts.size()) * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer[0] = g_theRenderer->CreateIndexBuffer(static_cast<unsigned int>(meshIndices.size()) * sizeof(unsigned int), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(meshVerts.data(), m_vertexBuffer[0]->GetSize(), m_vertexBuffer[0]);
	g_theRenderer->CopyCPUToGPU(meshIndices.data(), m_indexBuffer[0]->GetSize(), m_indexBuffer[0]);

	// Create buffers BLACK and copy to GPU
	m_vertexBuffer[1] = g_theRenderer->CreateVertexBuffer(static_cast<unsigned int>(meshVerts.size()) * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer[1] = g_theRenderer->CreateIndexBuffer(static_cast<unsigned int>(meshIndices.size()) * sizeof(unsigned int), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(meshVerts.data(), m_vertexBuffer[1]->GetSize(), m_vertexBuffer[1]);
	g_theRenderer->CopyCPUToGPU(meshIndices.data(), m_indexBuffer[1]->GetSize(), m_indexBuffer[1]);
}
