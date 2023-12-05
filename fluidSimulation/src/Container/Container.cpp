#include "Container.h"

#include "../Mesh/Cube/Cube.h"
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../ShaderProgram/ShaderProgram.h"

Container::Container()
{
    mMesh = new Cube();
    mTransform.SetWorldRotation(QVector3D(0,0,20));
    mTransform.SetWorldScale(QVector3D(30,1,30));
    mRenderShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    mCollider = new CubeCollider(&mTransform);
}

Container::~Container()
{
    delete mMesh;
}

void Container::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    //Shader
    mRenderShader->BindProgram();

    //View Projection matrix
    mRenderShader->SendMVP(mTransform.GetModelMatrix().data(), _viewMatrix, _projectionMatrix);
    mMesh->DrawMesh();

    //Shader
    mRenderShader->UnbindProgram();

    mCollider->Render();
}
