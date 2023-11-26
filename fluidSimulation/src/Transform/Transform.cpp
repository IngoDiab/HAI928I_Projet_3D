#include "Transform.h"
#include <QMatrix4x4>

Transform::Transform()
{

}

QMatrix4x4 Transform::GetModelMatrix() const
{
    QMatrix4x4 _modelMatrix;
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(mPosition);
    _modelMatrix.rotate(mRotation.z(), 0, 0, 1);
    _modelMatrix.rotate(mRotation.x(), 1, 0, 0);
    _modelMatrix.rotate(mRotation.y(), 0, 1, 0);
    _modelMatrix.scale(mScale);
    return _modelMatrix;
}
