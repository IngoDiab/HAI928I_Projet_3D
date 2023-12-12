#include "Container.h"

#include "../Mesh/Cube/Cube.h"
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../ShaderProgram/ShaderProgram.h"

Container::Container()
{
    mMesh = new Cube();
    mContainerShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    mCollider = new CubeCollider(&mTransform);
}

Container::~Container()
{
    delete mMesh;
}

void Container::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    //Shader
    mContainerShader->BindProgram();

    //View Projection matrix
    mContainerShader->SendMVP(mTransform.GetModelMatrix().data(), _viewMatrix, _projectionMatrix);
    mMesh->DrawMesh();

    //Shader
    mContainerShader->UnbindProgram();

    mCollider->Render();
}
