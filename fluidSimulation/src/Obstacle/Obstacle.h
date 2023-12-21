#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "../Transform/Transform.h"
#include <GL/gl.h>

class Cube;
class CubeCollider;
class ShaderProgram;

class Obstacle
{
    ShaderProgram* mContainerShader = nullptr;
    Cube* mMesh = nullptr;
    CubeCollider* mCollider = nullptr;
    Transform mTransform = Transform();

public:
    Cube* GetMesh() const { return mMesh; }
    CubeCollider* GetCollider() const {return mCollider;}
    Transform GetTransform() const { return mTransform; }
    Transform& GetTransform() { return mTransform; }

public:
    Obstacle();
    ~Obstacle();

public:
    void Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;

};

#endif // OBSTACLE_H
