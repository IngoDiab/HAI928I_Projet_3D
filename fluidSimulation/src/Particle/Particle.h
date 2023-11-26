#ifndef PARTICLE_H
#define PARTICLE_H

#include "../Transform/Transform.h"

class MyMesh;

class Particle
{
    //Display
    MyMesh* mMesh = nullptr;
    Transform mTransform = Transform();

public:
    MyMesh* GetMesh() const { return mMesh; }
    Transform GetTransform() const { return mTransform; }
    Transform& GetTransform() { return mTransform; }

public:
    Particle();
    Particle(const QVector3D& _position, const float _scale);
    ~Particle();
};

#endif // PARTICLE_H
