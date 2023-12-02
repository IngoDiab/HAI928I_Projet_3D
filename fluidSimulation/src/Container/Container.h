#ifndef CONTAINER_H
#define CONTAINER_H

#include "../Transform/Transform.h"
#include <GL/gl.h>

class Cube;
class CubeCollider;
class ShaderProgram;

class Container
{
    ShaderProgram* mRenderShader = nullptr;
    Cube* mMesh = nullptr;
    CubeCollider* mCollider = nullptr;
    Transform mTransform = Transform();

public:
    Cube* GetMesh() const { return mMesh; }
    Transform GetTransform() const { return mTransform; }
    Transform& GetTransform() { return mTransform; }

public:
    Container();
    ~Container();

public:
    void Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;

};

#endif // CONTAINER_H
