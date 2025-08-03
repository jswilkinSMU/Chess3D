#include "Game/ChessObject.hpp"
#include "Engine/Math/Mat44.hpp"

ChessObject::ChessObject()
{
}

ChessObject::~ChessObject()
{
}

Mat44 ChessObject::GetModelToWorldTransform() const
{
	Mat44 modelToWorldMatrix;
	modelToWorldMatrix.SetTranslation3D(m_position);
	modelToWorldMatrix.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
	return modelToWorldMatrix;
}
