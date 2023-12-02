#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

Fluid::Fluid(unsigned int _nbParticles)
{
    mGrid = new Grid();

    mParticleTemplateDisplay = new Particle();

    mVoxelsBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mParticlesBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);

    mNbParticles = _nbParticles;
    mParticleComputableData = QVector<ParticleComputableData>(_nbParticles);
    srand(time(NULL));

    for(ParticleComputableData& _particleData : mParticleComputableData)
    {
        _particleData.mPositionX = rand()/(float)RAND_MAX *20. -10;
        _particleData.mPositionY = rand()/(float)RAND_MAX *20. +10;
        _particleData.mPositionZ = rand()/(float)RAND_MAX *20. -10;
    }

    ComputeCenter();

    LoadShader();
}

Fluid::~Fluid()
{
    delete mGrid;
    delete mRenderShader;
    delete mComputeShaderForces;
    delete mComputeShaderGrid;
}

void Fluid::RefreshGrid(unsigned int _maxWorkGroupX, unsigned short _maxWorkGroupY, unsigned short _maxWorkGroupZ)
{
    float _bbX = numeric_limits<float>::max(), _bbY = numeric_limits<float>::max(), _bbZ = numeric_limits<float>::max();
    float _BBX = numeric_limits<float>::lowest(), _BBY = numeric_limits<float>::lowest(), _BBZ = numeric_limits<float>::lowest();

    for(const ParticleComputableData& _particleData : mParticleComputableData)
    {
        _bbX = min(_bbX, _particleData.mPositionX);
        _BBX = max(_BBX, _particleData.mPositionX);

        _bbY = min(_bbY, _particleData.mPositionY);
        _BBY = max(_BBY, _particleData.mPositionY);

        _bbZ = min(_bbZ, _particleData.mPositionZ);
        _BBZ = max(_BBZ, _particleData.mPositionZ);
    }

    int _sizeGrid = 7;
    QVector3D _bb = QVector3D(_bbX, _bbY, _bbZ);
    QVector3D _BB = QVector3D(_BBX, _BBY, _BBZ);
    mGrid->GenerateGrid(_bb, _BB, _sizeGrid);

//    for(unsigned int i = 0; i < mNbParticles; ++i)
//        mGrid->PutInVoxels(mParticleComputableData[i],i);

    if(!mComputeShaderGrid || mComputeShaderGrid->GetType() != SHADER_TYPE::COMPUTING) return;

    //Use compute shader
    mComputeShaderGrid->BindProgram();

    //Send uniforms
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniform1ui(mComputeShaderGrid->GetNbParticuleLocation(), mParticleComputableData.size());
    _functions.glUniform1ui(mComputeShaderGrid->GetGridSizeLocation(), _sizeGrid);
    for(ushort i = 0; i<3; ++i)
    {
        _functions.glUniform1f(mComputeShaderGrid->GetStepLocation(i), mGrid->GetStep(i));
        _functions.glUniform1f(mComputeShaderGrid->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Copy data
    mVoxelsBuffer.CopyDataToBuffer(mGrid->GetAllVoxels());
    mVoxelsBuffer.BindBase(0);
    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);
    mParticlesBuffer.BindBase(1);

    //Use CS
    mComputeShaderGrid->ProcessComputeShader(_sizeGrid,_sizeGrid,_sizeGrid);

    //Retrieve data from buffers
    Voxel* _retrievedVoxels = mVoxelsBuffer.RetrieveFromComputeShader<Voxel>();

    //Refresh voxels
    uint _nbVoxels = mGrid->GetNbVoxels();
    for(unsigned int i = 0; i < _nbVoxels; ++i)
        mGrid->GetVoxel(i) = _retrievedVoxels[i];

    //Shader
    mComputeShaderGrid->UnbindProgram();

//    QVector<uint> _voxels = mGrid->GetVoxelIndicesInRange(QVector3D(8,8,8), 8);
//    for(uint i = 0; i < _voxels.size(); ++i)
//        mGrid->GetVoxel(_voxels[i]).mNbParticles = 1;

    //Draw
    mGrid->DrawGrid();
}

void Fluid::ApplyForceOnCS(unsigned int _maxWorkGroupX, unsigned short _maxWorkGroupY, unsigned short _maxWorkGroupZ)
{
    if(!mComputeShaderForces || mComputeShaderForces->GetType() != SHADER_TYPE::COMPUTING) return;

    //Use compute shader
    mComputeShaderForces->BindProgram();

    //Copy and bind buffer to compute shader
    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);
    mParticlesBuffer.BindBase(0);

    //Use compute
    mComputeShaderForces->ProcessComputeShader(mParticleComputableData.size(),1,1);

    //Retrieve data from buffers
    ParticleComputableData* _retrievedParticles = mParticlesBuffer.RetrieveFromComputeShader<ParticleComputableData>();

    //Get back data on particles
    unsigned long _nbParticles = mParticleComputableData.size();
    for(unsigned long i = 0; i < _nbParticles; ++i)
        mParticleComputableData[i] = _retrievedParticles[i];

    //Shader
    mComputeShaderForces->UnbindProgram();
}

void Fluid::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles || !mParticleTemplateDisplay) return;

    //Shader
    mRenderShader->BindProgram();

    //int i = 0;
    //View Projection matrix
    mRenderShader->SendVP(_viewMatrix, _projectionMatrix);
    for(const ParticleComputableData& _particleData : mParticleComputableData)
    {
        //if(i%1000!=0)continue;
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

    //Shader
    mRenderShader->UnbindProgram();
}

void Fluid::LoadShader()
{
    mRenderShader = new ShaderProgram("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
    mComputeShaderForces = new ShaderProgram("src/shaders/compute_shader_forces.glsl");
    mComputeShaderGrid = new ShaderProgram("src/shaders/compute_shader_grid.glsl");
}

void Fluid::ComputeCenter()
{
    //mCenter = mParticles[0].GetTransform().GetWorldPosition();
    mCenter = QVector3D(0,0,0);
    for(const ParticleComputableData& _particleData : mParticleComputableData)
        mCenter += QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
    mCenter /= (float)mParticleComputableData.size();
}
