#include "PhysicManager.h"
#include "../Colliders/Cube/CubeCollider.h"

PhysicManager::PhysicManager()
{

}

QVector<CubeCollider> PhysicManager::GetCubeCollidersData() const
{
    QVector<CubeCollider> _cubes = QVector<CubeCollider>();
    for(CubeCollider* _collider : mCubePhysics)
        _cubes.push_back(*_collider);
    return _cubes;
}

void PhysicManager::AddCollider(CubeCollider* const _collider)
{
    if(Contains(_collider)) return;
    mCubePhysics.push_back(_collider);
}

bool PhysicManager::Contains(const CubeCollider* const _collider) const
{
    for(const CubeCollider* _colliderRegistered : mCubePhysics)
        if(_colliderRegistered == _collider) return true;
    return false;
}
