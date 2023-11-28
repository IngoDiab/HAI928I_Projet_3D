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

ParticleComputableData Particle::GetComputableData() const
{
    QVector3D _position = mTransform.GetWorldPosition();
    return ParticleComputableData(_position, mVelocity);
}

void Particle::RefreshComputableData(const ParticleComputableData& _processedComputableData)
{
    mVelocity = _processedComputableData.mVelocity;
    mTransform.SetWorldPosition(_processedComputableData.mPosition);
}
