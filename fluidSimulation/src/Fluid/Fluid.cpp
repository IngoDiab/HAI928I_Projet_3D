#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

Fluid::Fluid(unsigned long _nbParticles)
{
    mNbParticles = _nbParticles;
    mParticles = vector<Particle>(_nbParticles);
    srand(time(NULL));
    for(Particle& _particle : mParticles)
    {
        float _x = rand()/(float)RAND_MAX *5. -2.5;
        float _y = rand()/(float)RAND_MAX *5. -2.5;
        float _z = rand()/(float)RAND_MAX *5. -2.5;
        QVector3D _randomPosition = QVector3D(_x, _y, _z);


        _particle.GetTransform().SetWorldPosition(QVector3D(_randomPosition));
        _particle.GetTransform().SetWorldScale(QVector3D(mScaleParticle,mScaleParticle,mScaleParticle));
    }
    ComputeCenter();

    LoadShader();
}

Fluid::~Fluid()
{
    delete mShader;
}

void Fluid::RenderFluid(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles) return;

    //Shader
    mShader->BindProgram();

    //View Projection matrix
    mShader->SendVP(_viewMatrix, _projectionMatrix);

    for(const Particle& _particle : mParticles)
    {
        //Model matrix
        GLfloat* _modelMatrixF = _particle.GetTransform().GetModelMatrix().data();
        mShader->SendM(_modelMatrixF);

        //Draw
        MyMesh* _mesh = _particle.GetMesh();
        if(!_mesh) return;
        _mesh->DrawMesh();
    }
}

void Fluid::LoadShader()
{
    mShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
}

void Fluid::ComputeCenter()
{
    //mCenter = mParticles[0].GetTransform().GetWorldPosition();
    mCenter = QVector3D(0,0,0);
    for(const Particle& _particle : mParticles)
        mCenter += _particle.GetTransform().GetWorldPosition();
    mCenter /= (float)mParticles.size();
}
