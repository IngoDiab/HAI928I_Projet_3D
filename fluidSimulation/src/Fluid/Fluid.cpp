#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../Physics/PhysicManager/PhysicManager.h"

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
        _particleData.mPositionY = rand()/(float)RAND_MAX *20. +20;
        _particleData.mPositionZ = rand()/(float)RAND_MAX *20. -10;
    }

    ComputeCenter();

    LoadShader();
    RefreshGrid(0, 0, 0);
}

Fluid::~Fluid()
{
    delete mGrid;
    delete mRenderShader;
    delete mComputeShaderForces;
    delete mComputeShaderGrid;
}

void Fluid::Collisions()
{
    QVector<CubeCollider*> _allObstacles = PhysicManager::Instance()->GetCubeColliders();
//    QVector<Voxel> _voxels = mGrid->GetAllVoxels();
    for(ParticleComputableData& _particleData : mParticleComputableData)
    {
//        QVector3D _position = QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
//        QVector<uint> _voxelsNear = mGrid->GetVoxelIndicesInRange(_position, 10);
//        for(uint _index : _voxelsNear)
//        {
//            uint* _colliders = _voxels[_index].mCubeCollider;
//            uint _nbCollidersIn = _voxels[_index].mNbCubeCollider;

//            for(uint i = 0; i < _nbCollidersIn; ++i)
//            {
//                bool _isColliding = Detection(_position, *_allObstacles[_colliders[i]]);
//                if(_isColliding) Resolution(_particleData, *_allObstacles[_colliders[i]]);
//            }
//        }
        for(CubeCollider* _obstacle : _allObstacles)
        {
            QVector3D _position = QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
            bool _isColliding = Detection(_position, *_obstacle);
            if(_isColliding) Resolution(_particleData, *_obstacle);
        }
    }
}

bool Fluid::Detection(const QVector3D& _position, const CubeCollider& _collider)
{
    //Get bb
    QVector3D _colliderbb = _collider.Getbb();

    //Get dimensions cube
    float _sizeX = _collider.GetXAxisCollisionLength();
    float _sizeY = _collider.GetYAxisCollisionLength();
    float _sizeZ = _collider.GetZAxisCollisionLength();

    //Get relatives axis cube
    QVector3D _right = _collider.GetXAxisCollision();
    QVector3D _up = _collider.GetYAxisCollision();
    QVector3D _forward = _collider.GetZAxisCollision();

    //Project particle on all axis
    QVector3D _bbToProjOnRight = (QVector3D::dotProduct(_position - _colliderbb, _right) * _right);
    QVector3D _bbToProjOnUp = (QVector3D::dotProduct(_position - _colliderbb, _up) * _up);
    QVector3D _bbToProjOnForward = (QVector3D::dotProduct(_position - _colliderbb, _forward) * _forward );

    //Get distance particle from bb and check if <= size
    float _distanceX = _bbToProjOnRight.length();
    float _distanceY = _bbToProjOnUp.length();
    float _distanceZ = _bbToProjOnForward.length();
    if(_distanceX > _sizeX || _distanceY > _sizeY || _distanceZ > _sizeZ) return false;

    //Check same orientation
    float _dotX = QVector3D::dotProduct(_bbToProjOnRight.normalized(), _right);
    float _dotY = QVector3D::dotProduct(_bbToProjOnUp.normalized(), _up);
    float _dotZ = QVector3D::dotProduct(_bbToProjOnForward.normalized(), _forward);
    if(_dotX < 0 || _dotY < 0 || _dotZ < 0) return false;

    //If same orientation and bb to projection <= size on dimension for EVERY axis
    return true;
}

void Fluid::Resolution(ParticleComputableData& _particle, const CubeCollider& _collider)
{
    QVector3D _previousPos = QVector3D(_particle.mPreviousPositionX, _particle.mPreviousPositionY, _particle.mPreviousPositionZ);
    QVector3D _currentPos = QVector3D(_particle.mPositionX, _particle.mPositionY, _particle.mPositionZ);
    QVector3D _movementDir = (_currentPos - _previousPos).normalized();

    //Get bb & BB
    QVector<QVector3D> _corners = QVector<QVector3D>(2);
    _corners[0] = _collider.Getbb();
    _corners[1] = _collider.GetBB();

    //Get relatives axis cube
    QVector<QVector3D> _normales = QVector<QVector3D>(3);
    _normales[0] = _collider.GetXAxisCollision();
    _normales[1] = _collider.GetYAxisCollision();
    _normales[2] = _collider.GetZAxisCollision();

    float _minT = numeric_limits<float>::max();

    for(ushort i = 0; i < 6; ++i)
    {
        QVector3D _corner = _corners[i/3];
        QVector3D _normalPlane = (i<3 ? 1 : -1) * _normales[i%3];

        //Check plan not parallel to velocity
        float _dotVeloNormal = QVector3D::dotProduct(_movementDir, _normalPlane);
        if((_dotVeloNormal <= 0.01f  && _dotVeloNormal >= -0.01f) || !isfinite(_dotVeloNormal)) continue;

        float _t = QVector3D::dotProduct((_corner - _previousPos), _normalPlane) / QVector3D::dotProduct(_movementDir, _normalPlane);
        if(_t < -0.01f) continue;

        _minT = min(_minT, _t);
    }

    QVector3D _newPos = _previousPos + (_minT) * _movementDir;
    _particle.mPositionX = _newPos.x();
    _particle.mPositionY = _newPos.y();
    _particle.mPositionZ = _newPos.z();

    _particle.mVelocityY = -_particle.mVelocityY/2;

    //qDebug() << _newPos;
}

void Fluid::UpdateFluid(double _deltaTime, unsigned int _maxWorkGroupX, unsigned short _maxWorkGroupY, unsigned short _maxWorkGroupZ)
{
    mTimer+=_deltaTime;
    float _dtPhysic = 1.0f/(float)mFPSFluid;
    if(mTimer < _dtPhysic) return;
    mTimer = 0;

    ApplyForceOnCS(_maxWorkGroupX, _maxWorkGroupY, _maxWorkGroupZ);
    Collisions();
    RefreshGrid(_maxWorkGroupX, _maxWorkGroupY, _maxWorkGroupZ);
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

    //Draw
    mGrid->DrawGrid();
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
    mGrid->GenerateBoundingBoxGrid(_bb, _BB, _sizeGrid);

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

    QVector<CubeCollider*> _colliders = PhysicManager::Instance()->GetCubeColliders();
    for(int i = 0; i < _colliders.size(); ++i)
        mGrid->PutInVoxels(*_colliders[i],i);
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
