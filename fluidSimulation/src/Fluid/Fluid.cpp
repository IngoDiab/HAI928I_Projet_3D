#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

Fluid::Fluid(unsigned long _nbParticles)
{
    mParticleTemplateDisplay = new Particle();

    mParticlesBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);

    mNbParticles = _nbParticles;
    //mParticles = QVector<Particle>(_nbParticles);
    mParticleComputableData = QVector<ParticleComputableData>(_nbParticles);
    srand(time(NULL));
    for(ParticleComputableData& _particleData : mParticleComputableData)
    {
        float _x = rand()/(float)RAND_MAX *5. -2.5;
        float _y = rand()/(float)RAND_MAX *5. -2.5;
        float _z = rand()/(float)RAND_MAX *5. -2.5;
        //QVector3D _randomPosition = QVector3D(_x, _y, _z);


//        _particle.GetTransform().SetWorldPositimScaleParticleon(QVector3D(_randomPosition));
//        _particle.GetTransform().SetWorldScale(QVector3D(mScaleParticle,mScaleParticle,mScaleParticle));
        _particleData.mPositionX = _x;
        _particleData.mPositionY = _y;
        _particleData.mPositionZ = _z;
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

    //Use compute shader
    mComputeShader->BindProgram();

    //Copy and bind buffer to compute shader
    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);
    mParticlesBuffer.BindBase(0);

    //Use compute
    ParticleComputableData* _processedData = mComputeShader->ProcessComputeShader<ParticleComputableData>(10000,1,1, mParticlesBuffer);
    mComputeShader->UnbindProgram();

    //Get back data on particles
    unsigned long _nbParticles = mParticleComputableData.size();
    for(unsigned long i = 0; i < _nbParticles; ++i)
        mParticleComputableData[i] = _processedData[i];
}

void Fluid::RenderFluid(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles || !mParticleTemplateDisplay) return;

    //Shader
    mRenderShader->BindProgram();

    //View Projection matrix
    mRenderShader->SendVP(_viewMatrix, _projectionMatrix);

    for(const ParticleComputableData& _particleData : mParticleComputableData)
    {
        mParticleTemplateDisplay->GetTransform().SetWorldPosition(QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ));
        mParticleTemplateDisplay->SetVelocity(QVector3D(_particleData.mVelocityX, _particleData.mVelocityY, _particleData.mVelocityZ));

        //Model matrix
        GLfloat* _modelMatrixF = mParticleTemplateDisplay->GetTransform().GetModelMatrix().data();
        mRenderShader->SendM(_modelMatrixF);

        //Draw
        MyMesh* _mesh = mParticleTemplateDisplay->GetMesh();
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
    for(const ParticleComputableData& _particleData : mParticleComputableData)
        mCenter += QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
    mCenter /= (float)mParticleComputableData.size();
}
