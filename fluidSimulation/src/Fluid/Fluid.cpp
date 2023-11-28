#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

Fluid::Fluid(unsigned long _nbParticles)
{
    mParticlesBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);

    mNbParticles = _nbParticles;
    mParticles = QVector<Particle>(_nbParticles);
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
    delete mRenderShader;
    delete mComputeShader;
}

void Fluid::ProcessFluid()
{
    if(!mComputeShader || mComputeShader->GetType() != SHADER_TYPE::COMPUTING) return;

    //Get computables data
    unsigned long _nbParticles = mParticles.size();
    mParticleComputableData.clear();
    mParticleComputableData.resize(_nbParticles);
    for(unsigned long i = 0; i < _nbParticles; ++i)
        mParticleComputableData[i] = mParticles[i].GetComputableData();

    //Use compute shader
    mComputeShader->BindProgram();

    //Copy and bind buffer to compute shader
    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);
    mParticlesBuffer.BindBase(0);

    //Use compute
    qDebug() << "0 " <<mParticleComputableData[0].mPosition;
    qDebug() << "1 " <<mParticleComputableData[1].mPosition;
    qDebug() << "2 " <<mParticleComputableData[2].mPosition;
    qDebug() << "3 " <<mParticleComputableData[3].mPosition;
    qDebug() << "4 " <<mParticleComputableData[4].mPosition;
    ParticleComputableData* _processedData = mComputeShader->ProcessComputeShader<ParticleComputableData>(5,1,1, mParticlesBuffer);
    mComputeShader->UnbindProgram();

    //Get back data on particles
    for(unsigned long i = 0; i < _nbParticles; ++i)
        mParticles[i].RefreshComputableData(_processedData[i]);
}

void Fluid::RenderFluid(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles) return;

    //Shader
    mRenderShader->BindProgram();

    //View Projection matrix
    mRenderShader->SendVP(_viewMatrix, _projectionMatrix);

    for(const Particle& _particle : mParticles)
    {
        //Model matrix
        GLfloat* _modelMatrixF = _particle.GetTransform().GetModelMatrix().data();
        mRenderShader->SendM(_modelMatrixF);

        //Draw
        MyMesh* _mesh = _particle.GetMesh();
        if(!_mesh) return;
        _mesh->DrawMesh();
    }
}

void Fluid::LoadShader()
{
    mRenderShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    mComputeShader = new ShaderProgram("src/shaders/compute_shader.glsl");
}

void Fluid::ComputeCenter()
{
    //mCenter = mParticles[0].GetTransform().GetWorldPosition();
    mCenter = QVector3D(0,0,0);
    for(const Particle& _particle : mParticles)
        mCenter += _particle.GetTransform().GetWorldPosition();
    mCenter /= (float)mParticles.size();
}
