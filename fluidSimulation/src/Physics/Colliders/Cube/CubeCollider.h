#ifndef CUBECOLLIDER_H
#define CUBECOLLIDER_H

#include <GL/gl.h>
#include <QVector3D>
#include <QVector>

class Transform;

class CubeCollider
{
    const Transform* mParent = nullptr;
    QVector<QVector3D> mPositions = QVector<QVector3D>();

public:
    const Transform* GetParentTransform() const {return mParent;}
    QVector3D Getbb() const {return mPositions[0];}
    QVector3D GetBB() const {return mPositions[6];}

    QVector3D GetXAxisCollision() const {return (mPositions[1] - mPositions[0]).normalized();}
    QVector3D GetOtherXCorner() const {return mPositions[1];}

    QVector3D GetYAxisCollision() const {return (mPositions[5] - mPositions[0]).normalized();}
    QVector3D GetOtherYCorner() const {return mPositions[5];}

    QVector3D GetZAxisCollision() const {return (mPositions[2] - mPositions[0]).normalized();}
    QVector3D GetOtherZCorner() const {return mPositions[2];}

    float GetXAxisCollisionLength() const {return (mPositions[1] - mPositions[0]).length();}
    float GetYAxisCollisionLength() const {return (mPositions[5] - mPositions[0]).length();}
    float GetZAxisCollisionLength() const {return (mPositions[2] - mPositions[0]).length();}
public:
    CubeCollider(const Transform* _transform);

public:
    void DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3) const;
    void Render() const;
};

#endif // CUBECOLLIDER_H
