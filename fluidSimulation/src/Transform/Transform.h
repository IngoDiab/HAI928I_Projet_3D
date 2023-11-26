#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>

class Transform
{
    QVector3D mPosition = QVector3D(1,1,1);
    QVector3D mRotation = QVector3D();
    QVector3D mScale = QVector3D(1,1,1);

public:
    QVector3D GetWorldPosition() const { return mPosition;}
    QVector3D GetWorldRotation() const { return mRotation;}
    QVector3D GetWorldScale() const { return mScale;}

    void SetWorldPosition(const QVector3D& _position) { mPosition = _position;}
    void SetWorldRotation(const QVector3D& _rotation) { mRotation = _rotation;}
    void SetWorldScale(const QVector3D& _scale) { mScale = _scale;}

public:
    Transform();

public:
    QMatrix4x4 GetModelMatrix() const;
};

#endif // TRANSFORM_H
