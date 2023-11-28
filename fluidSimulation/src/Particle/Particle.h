#ifndef PARTICLE_H
#define PARTICLE_H

#include "../Transform/Transform.h"

class MyMesh;

struct ParticleComputableData
{
    QVector3D mPosition = QVector3D(0,0,0);
    QVector3D mVelocity = QVector3D(0,0,0);

    ParticleComputableData(){}
    ParticleComputableData(const QVector3D _position, const QVector3D& _velocity) : mPosition(_position), mVelocity(_velocity){}
};

class Particle
{
    //Display
    MyMesh* mMesh = nullptr;

    //Data
    Transform mTransform = Transform();
    QVector3D mVelocity = QVector3D(0,0,0);

public:
    MyMesh* GetMesh() const { return mMesh; }
    Transform GetTransform() const { return mTransform; }
    Transform& GetTransform() { return mTransform; }

public:
    Particle();
    Particle(const QVector3D& _position, const float _scale);
    ~Particle();

public:
    ParticleComputableData GetComputableData() const;
    void RefreshComputableData(const ParticleComputableData& _processedComputableData);
};

#endif // PARTICLE_H
