#include "Container.h"

#include "../Mesh/Cube/Cube.h"
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../ShaderProgram/ShaderProgram.h"

Container::Container()
{
    mMesh = new Cube();
    mCollider = new CubeCollider();
    mTransform.SetWorldScale(QVector3D(30,1,30));
    mRenderShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
}

Container::~Container()
{
    delete mMesh;
}

void Container::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    //Shader
    mRenderShader->BindProgram();

    //int i = 0;
    //View Projection matrix
    mRenderShader->SendMVP(mTransform.GetModelMatrix().data(), _viewMatrix, _projectionMatrix);
    mMesh->DrawMesh();

    //Shader
    mRenderShader->UnbindProgram();
}
