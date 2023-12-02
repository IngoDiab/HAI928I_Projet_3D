#ifndef CUBECOLLIDER_H
#define CUBECOLLIDER_H

#include <QVector3D>
#include <QVector>

class CubeCollider
{
    QVector<QVector3D> mPositions = QVector<QVector3D>();

public:
    CubeCollider();
};

#endif // CUBECOLLIDER_H
