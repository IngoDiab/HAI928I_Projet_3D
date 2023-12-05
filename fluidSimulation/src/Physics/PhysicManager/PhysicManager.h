#ifndef PHYSICMANAGER_H
#define PHYSICMANAGER_H

#include <QVector>

#include "../../Utils/Singleton.h"
class CubeCollider;

class PhysicManager : public Singleton<PhysicManager>
{
    QVector<CubeCollider*> mCubePhysics = QVector<CubeCollider*>();

public:
    QVector<CubeCollider*> GetCubeColliders() const {return mCubePhysics;}

public:
    PhysicManager();

public:
    void AddCollider(CubeCollider* const _collider);

private:
    bool Contains(const CubeCollider* const _collider) const;
};

#endif // PHYSICMANAGER_H
