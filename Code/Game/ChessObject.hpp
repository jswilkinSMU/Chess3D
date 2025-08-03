#pragma once
#include "Engine/Math/Vec3.h"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.h"
#include "Engine/Math/IntVec2.h"
// -----------------------------------------------------------------------------
class Game;
class Mat44;
class ChessMatch;
class ChessPiece;
class ChessObject;
// -----------------------------------------------------------------------------
struct ChessRaycastResult
{
public:
	Vec3  m_rayStartPosition = Vec3::ZERO;
	Vec3  m_rayFwdNormal = Vec3::ZERO;
	float m_rayLength = 20.0f;

	IntVec2 m_coords = IntVec2::ZERO;
	ChessPiece* m_chessPiece = nullptr;
	ChessObject* m_chessObject = nullptr;

	bool m_didImpact = false;
	Vec3 m_impactPos = Vec3::ZERO;
	Vec3 m_impactNormal = Vec3::ZAXE;
	float m_impactDist = 0.0f;

	void Reset()
	{
		m_coords = -IntVec2::ONE;
		m_chessPiece = nullptr;
		m_chessObject = nullptr;
		m_didImpact = false;
		m_impactPos = Vec3::ZERO;
		m_impactNormal = Vec3::ZAXE;
		m_impactDist = 0.0f;
	}
};
// -----------------------------------------------------------------------------
class ChessObject
{
public:
	ChessObject();
	virtual ~ChessObject();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual Mat44 GetModelToWorldTransform() const;
	virtual void RaycastVsMe(ChessRaycastResult& result) const = 0;

public:
	ChessMatch* m_match = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
};