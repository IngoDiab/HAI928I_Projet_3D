#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>
#include <QMatrix4x4>

class Transform
{
    mutable QVector3D mPreviousPosition = QVector3D();
    QVector3D mPosition = QVector3D();
    QVector3D mRotation = QVector3D();
    QVector3D mScale = QVector3D(1,1,1);

    QVector3D mVelocity = QVector3D();

public:
    QVector3D GetRightAxis() const {return GetModelMatrix()*QVector3D(1,0,0);}
    QVector3D GetUpAxis() const {return GetModelMatrix()*QVector3D(0,1,0);}
    QVector3D GetForwardAxis() const {return GetModelMatrix()*QVector3D(0,0,1);}

    QVector3D GetWorldPreviousPosition() const { return mPreviousPosition;}
    QVector3D GetWorldPosition() const { return mPosition;}
    QVector3D GetWorldRotation() const { return mRotation;}
    QVector3D GetWorldScale() const { return mScale;}

    void RegisterPosition() const { mPreviousPosition = mPosition;}
    void SetWorldPosition(const QVector3D& _position) { mPosition = _position;}
    void SetWorldRotation(const QVector3D& _rotation) { mRotation = _rotation;}
    void SetWorldScale(const QVector3D& _scale) { mScale = _scale;}

public:
    Transform();

public:
    QMatrix4x4 GetModelMatrix() const;
    QMatrix4x4 GetModelMatrixNoScale() const;
};

#endif // TRANSFORM_H
