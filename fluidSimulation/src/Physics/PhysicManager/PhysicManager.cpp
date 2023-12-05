#include "PhysicManager.h"
#include "../Colliders/Cube/CubeCollider.h"

PhysicManager::PhysicManager()
{

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
