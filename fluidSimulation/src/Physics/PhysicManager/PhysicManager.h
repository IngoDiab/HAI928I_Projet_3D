#ifndef PHYSICMANAGER_H
#define PHYSICMANAGER_H

#include <QVector>

#include "../../Utils/Singleton.h"
class CubeCollider;

class PhysicManager : public Singleton<PhysicManager>
{
    int mFPSPhysic = 200;
    float mTimer = 0;

    QVector<CubeCollider> mCubePhysics = QVector<CubeCollider>();

public:
    PhysicManager();

public:
    void UpdatePhysic(const float _deltaTime);
};

#endif // PHYSICMANAGER_H
