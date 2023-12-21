#ifndef PARTICLE_H
#define PARTICLE_H

#include "../Transform/Transform.h"

class MyMesh;

struct ParticleComputableData
{
    float mPreviousPositionX = 0, mPreviousPositionY = 0, mPreviousPositionZ = 0;
    float mPositionX = 0, mPositionY = 0, mPositionZ = 0;
    float mPredictedPositionX = 0, mPredictedPositionY = 0, mPredictedPositionZ = 0;
    float mVelocityX = 0, mVelocityY = 0, mVelocityZ = 0;
    float mDensity = 0;
    float mPressure = 0;
//    float mPressureForceX = 0, mPressureForceY = 0, mPressureForceZ = 0;
//    float mViscosityForceX = 0, mViscosityForceY = 0, mViscosityForceZ = 0;

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
