#include "Obstacle.h"

#include "../Mesh/Cube/Cube.h"
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../ShaderProgram/ShaderProgram.h"

Obstacle::Obstacle()
{
    mMesh = new Cube();
    mContainerShader = new ShaderProgram("src/shaders/Vertex/vertex_shader.glsl", "src/shaders/Fragments/fragment_shader.glsl");
    mCollider = new CubeCollider(&mTransform);
}

Obstacle::~Obstacle()
{
    delete mMesh;
}

void Obstacle::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    //Shader
    mContainerShader->BindProgram();

    //View Projection matrix
    mContainerShader->SendMVP(mTransform.GetModelMatrix().data(), _viewMatrix, _projectionMatrix);
    mMesh->DrawMesh();

    //Shader
    mContainerShader->UnbindProgram();

    //mCollider->Render();
}
