#ifndef CUBECOLLIDER_H
#define CUBECOLLIDER_H

#include <GL/gl.h>
#include <QVector3D>
#include <QVector>

class Transform;

class CubeCollider
{
    const Transform* mParent = nullptr;
    float mVelocityX = 0, mVelocityY = 0, mVelocityZ = 0;
    float mCorners[24] = { 0 };

public:
    const Transform* GetParentTransform() const {return mParent;}
    QVector3D GetVelocity() const { return QVector3D(mVelocityX, mVelocityY, mVelocityZ);}
    void ResetVelocity() { mVelocityX = 0, mVelocityY = 0, mVelocityZ = 0;}
    float* GetCorners() {return mCorners;}

    QVector3D GetCorner(uint _indexCorner) const {return QVector3D(mCorners[_indexCorner*3], mCorners[_indexCorner*3+1], mCorners[_indexCorner*3+2]);}

    QVector3D Getbb() const {return GetCorner(0);}
    QVector3D GetBB() const {return GetCorner(6);}

    QVector3D GetXAxisCollision() const {return (GetCorner(1) - GetCorner(0)).normalized();}
    QVector3D GetYAxisCollision() const {return (GetCorner(5) - GetCorner(0)).normalized();}
    QVector3D GetZAxisCollision() const {return (GetCorner(2) - GetCorner(0)).normalized();}

    float GetXAxisCollisionLength() const {return (GetCorner(1) - GetCorner(0)).length();}
    float GetYAxisCollisionLength() const {return (GetCorner(5) - GetCorner(0)).length();}
    float GetZAxisCollisionLength() const {return (GetCorner(2) - GetCorner(0)).length();}
public:
    CubeCollider(const Transform* _transform);

public:
    void RefreshColliderTransform();
    void CalculateDeplacement();
    void DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3) const;
    void Render() const;
};

#endif // CUBECOLLIDER_H
