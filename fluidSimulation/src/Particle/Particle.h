#ifndef PARTICLE_H
#define PARTICLE_H

#include "../Transform/Transform.h"

class MyMesh;

struct ParticleComputableData
{
    float mPositionX, mPositionY, mPositionZ = 0;
    float mVelocityX, mVelocityY, mVelocityZ = 0;

    ParticleComputableData(){}
    ParticleComputableData(const QVector3D _position, const QVector3D& _velocity)
    {
        mPositionX = _position.x();
        mPositionY = _position.y();
        mPositionZ = _position.z();

        mVelocityX = _velocity.x();
        mVelocityY = _velocity.y();
        mVelocityZ = _velocity.z();
    }
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

    void SetVelocity(const QVector3D& _velocity) { mVelocity = _velocity; }

public:
    Particle();
    Particle(const QVector3D& _position, const float _scale);
    ~Particle();

public:
    ParticleComputableData GetComputableData() const;
    void RefreshComputableData(const ParticleComputableData& _processedComputableData);
};

#endif // PARTICLE_H
