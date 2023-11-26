#include "Particle.h"
#include "../Mesh/Cube/Cube.h"
#include "../Mesh/Sphere/Sphere.h"

Particle::Particle()
{
    mMesh = new Sphere();
}

Particle::Particle(const QVector3D& _position, const float _scale)
{
    mMesh = new Sphere();
    mTransform.SetWorldPosition(_position);
    mTransform.SetWorldScale(QVector3D(_scale,_scale,_scale));
}

Particle::~Particle()
{
    delete mMesh;
}
