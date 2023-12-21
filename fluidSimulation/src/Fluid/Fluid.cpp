#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../Physics/PhysicManager/PhysicManager.h"

#include "../Utils/MarchingCube_Configurations.h"

Fluid::Fluid()
{
    mContext = QOpenGLExtraFunctions(QOpenGLContext::currentContext());

    mGrid = new Grid(5);
    mMarchingCubeGrid = new MCGrid(15);
    mMarchingCubeGrid->GenerateBoundingBoxGrid(QVector3D(-6.5f,0,-7), QVector3D(6.5f,13,7));

    mFluidMesh = new MyMesh();
    mParticleTemplateDisplay = new Particle();

    mGridBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mMarchingCubeGridBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mVoxelsIndicesParticlesBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mVoxelsIndicesCollidersBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mParticlesBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mCollidersBuffer.SetUsagePattern(QOpenGLBuffer::DynamicDraw);

    mBufferPresenceParticleInVoxels.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mMCTable.SetUsagePattern(QOpenGLBuffer::DynamicDraw);
    mMCEdgeTable.SetUsagePattern(QOpenGLBuffer::DynamicDraw);

    mParticleComputableData = QVector<ParticleComputableData>(mNbParticles);
    srand(time(NULL));

    for(ParticleComputableData& _particleData : mParticleComputableData)
    {
        _particleData.mPositionX = rand()/(float)RAND_MAX *8. -4;
        _particleData.mPositionY = rand()/(float)RAND_MAX *10. +2;
        _particleData.mPositionZ = rand()/(float)RAND_MAX *8. -4;
    }

    ComputeCenter();

    LoadShader();
}

Fluid::~Fluid()
{
    delete mFluidMesh;
    delete mParticleTemplateDisplay;

    delete mGrid;
    delete mMarchingCubeGrid;
    delete mFluidShader;
    delete mComputeShaderGravity;
    delete mComputeShaderDensityPressure;
    delete mComputeShaderGradient;
    delete mComputeShaderVelocity;
    delete mComputeShaderCollisions;
    delete mComputeShaderGrid;
}

void Fluid::Initialize()
{
    mVoxelsIndicesParticlesBuffer.CopyDataToBuffer(mGrid->GetAllVoxelsParticlesIndices());
    mVoxelsIndicesCollidersBuffer.CopyDataToBuffer(mGrid->GetAllVoxelsCubeCollidersIndices());
    mBufferPresenceParticleInVoxels.CopyDataToBuffer(QVector<uint>(mGrid->GetNbVoxels()*MAX_PARTICLES_PER_VOXEL, 0));
    mMCTable.CopyDataToBuffer(MCTable);
    mMCEdgeTable.CopyDataToBuffer(MCEdgeTable);

    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);

    GenerateGrid();
    RefreshGrid();
}

void Fluid::UpdateFluid(float _deltaTime)
{
    mTimer+=_deltaTime;
    float _dtPhysic = 1.0f/(float)mFPSFluid;
    if(mTimer < _dtPhysic) return;
    mTimer = 0;
    mDeltaTimePhysics = max(_dtPhysic,_dtPhysic);
    QVector<CubeCollider> _cubeColliders = PhysicManager::Instance()->GetCubeCollidersData();
    mCollidersBuffer.CopyDataToBuffer(_cubeColliders);
    Gravity();
    Density_Pressure();
    Gradient();
    Viscosity();
    Velocity();
    Collisions();
    GenerateGrid();
    RefreshGrid();
    MarchingCube();
    MergeTriangles();
}

/*void Fluid::Collisions()
{
    QVector<CubeCollider*> _allObstacles = PhysicManager::Instance()->GetCubeColliders();
    QVector<Voxel> _voxels = mGrid->GetAllVoxels();
    for(ParticleComputableData& _particleData : mParticleComputableData)
    {
//        QVector3D _position = QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
//        QVector<uint> _voxelsNear = mGrid->GetVoxelIndicesInRange(_position, 10);

//        QVector<uint> _alreadyProcessedCollision = QVector<uint>();
//        float _minT = numeric_limits<float>::max();
//        bool _needCollision = false;
//        for(uint _index : _voxelsNear)
//        {
//            //uint* _colliders = _voxels[_index].mCubeCollider;
//            uint _nbCollidersIn = _voxels[_index].mNbCubeCollider;

//            for(uint i = 0; i < _nbCollidersIn; ++i)
//            {
//                uint _indexCollider = mGrid->GetAllVoxelsCubeCollidersIndices()[_index*MAX_CUBE_COLLIDERS_PER_VOXEL+i];
//                if(_alreadyProcessedCollision.contains(_indexCollider)) continue;
//                _alreadyProcessedCollision.push_back(_indexCollider);
//                bool _isColliding = Detection(_position, *_allObstacles[_indexCollider]);
//                if(_isColliding)
//                {
//                    _needCollision = true;
//                    _minT = min(_minT, Resolution(_particleData, *_allObstacles[_indexCollider]));
//                }
//            }
//        }

//        if(_needCollision){
//        QVector3D _previousPos = QVector3D(_particleData.mPreviousPositionX, _particleData.mPreviousPositionY, _particleData.mPreviousPositionZ);
//        QVector3D _currentPos = QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
//        QVector3D _movementDir = (_currentPos - _previousPos).normalized();

//        QVector3D _newPos = _previousPos + _minT * _movementDir;
//        _particleData.mPositionX = _newPos.x();
//        _particleData.mPositionY = _newPos.y();
//        _particleData.mPositionZ = _newPos.z();

//        _particleData.mVelocityY = -_particleData.mVelocityY/2;
//        }
        for(CubeCollider* _obstacle : _allObstacles)
        {
            QVector3D _position = QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
            bool _isColliding = Detection(_position, *_obstacle);
            if(_isColliding) Resolution(_particleData, *_obstacle);
        }
    }
    mParticlesBuffer.CopyDataToBuffer(mParticleComputableData);
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

float Fluid::Resolution(ParticleComputableData& _particle, const CubeCollider& _collider)
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

    //return _minT;

    QVector3D _newPos = _previousPos + _minT * _movementDir;
    _particle.mPositionX = _newPos.x();
    _particle.mPositionY = _newPos.y();
    _particle.mPositionZ = _newPos.z();

    _particle.mVelocityY = -_particle.mVelocityY/2;
}*/

void Fluid::Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles || !mParticleTemplateDisplay) return;

    //Shader
    mFluidShader->BindProgram();

    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());

    //View Projection matrix
    mFluidShader->SendVP(_viewMatrix, _projectionMatrix);
//    for(int i = 0; i < mParticleComputableData.size(); ++i)
//    {
//        //if(i%100 !=0) continue;
//        const ParticleComputableData& _particleData = mParticleComputableData[i];
//        mParticleTemplateDisplay->GetTransform().SetWorldPosition(QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ));
//        mParticleTemplateDisplay->SetVelocity(QVector3D(_particleData.mVelocityX, _particleData.mVelocityY, _particleData.mVelocityZ));

//        _functions.glUniform1f(mFluidShader->GetDensityParticleLocation(), mParticleComputableData[i].mDensity);
//        _functions.glUniform1f(mFluidShader->GetPressureParticleLocation(), mParticleComputableData[i].mPressure);

//        //Model matrix
//        GLfloat* _modelMatrixF = mParticleTemplateDisplay->GetTransform().GetModelMatrix().data();
//        mFluidShader->SendM(_modelMatrixF);

//        //Draw
//        MyMesh* _mesh = mParticleTemplateDisplay->GetMesh();
//        if(!_mesh) return;
//        _mesh->DrawMesh();
//    }

    mFluidMesh->DrawMesh();

    //Shader
    mFluidShader->UnbindProgram();
}

void Fluid::Gravity()
{
    //Bind
    mComputeShaderGravity->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderGravity->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderGravity->GetDeltaTimeLocation(), mDeltaTimePhysics);

    //Bind buffer
    mParticlesBuffer.BindBase(3);

    //Dispatch
    mComputeShaderGravity->ProcessComputeShader(NB_PARTICLES,1,1);

    //Unbind
    mComputeShaderGravity->UnbindProgram();
}

void Fluid::Density_Pressure()
{
    //Bind
    mComputeShaderDensityPressure->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderDensityPressure->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderDensityPressure->GetDeltaTimeLocation(), mDeltaTimePhysics);
    mContext.glUniform1ui(mComputeShaderDensityPressure->GetGridSizeLocation(), mGrid->GetSize());
    for(ushort i = 0; i<3; ++i)
    {
        mContext.glUniform1f(mComputeShaderDensityPressure->GetStepLocation(i), mGrid->GetStep(i));
        mContext.glUniform1f(mComputeShaderDensityPressure->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Bind buffer
    mGridBuffer.BindBase(0);
    mVoxelsIndicesParticlesBuffer.BindBase(1);
    mParticlesBuffer.BindBase(3);

    //Dispatch
    mComputeShaderDensityPressure->ProcessComputeShader(NB_PARTICLES,1,1);

//    QVector<ParticleComputableData> _pa = QVector<ParticleComputableData>(10240);
//    mParticlesBuffer.ReadFromBuffer(0, &_pa[0], 10240*sizeof(ParticleComputableData));
//    qDebug() << "Post densite" << _pa[0].mDensity;

    //Unbind
    mComputeShaderDensityPressure->UnbindProgram();
}

void Fluid::Gradient()
{
    //Bind
    mComputeShaderGradient->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderGradient->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderGradient->GetDeltaTimeLocation(), mDeltaTimePhysics);
    mContext.glUniform1ui(mComputeShaderGradient->GetGridSizeLocation(), mGrid->GetSize());
    for(ushort i = 0; i<3; ++i)
    {
        mContext.glUniform1f(mComputeShaderGradient->GetStepLocation(i), mGrid->GetStep(i));
        mContext.glUniform1f(mComputeShaderGradient->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Bind buffer
    mGridBuffer.BindBase(0);
    mVoxelsIndicesParticlesBuffer.BindBase(1);
    mParticlesBuffer.BindBase(3);

//    QVector<ParticleComputableData> _pa = QVector<ParticleComputableData>(10240);
//    mParticlesBuffer.ReadFromBuffer(0, &_pa[0], 10240*sizeof(ParticleComputableData));
//    qDebug() << "Pre Gradient Y" << _pa[0].mPositionY;

    //Dispatch
    mComputeShaderGradient->ProcessComputeShader(NB_PARTICLES,1,1);

    //Unbind
    mComputeShaderGradient->UnbindProgram();
}

void Fluid::Viscosity()
{
    //Bind
    mComputeShaderViscosity->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderViscosity->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderViscosity->GetDeltaTimeLocation(), mDeltaTimePhysics);
    mContext.glUniform1ui(mComputeShaderViscosity->GetGridSizeLocation(), mGrid->GetSize());
    for(ushort i = 0; i<3; ++i)
    {
        mContext.glUniform1f(mComputeShaderViscosity->GetStepLocation(i), mGrid->GetStep(i));
        mContext.glUniform1f(mComputeShaderViscosity->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Bind buffer
    mGridBuffer.BindBase(0);
    mVoxelsIndicesParticlesBuffer.BindBase(1);
    mParticlesBuffer.BindBase(3);

    //Dispatch
    mComputeShaderViscosity->ProcessComputeShader(NB_PARTICLES,1,1);

    //Unbind
    mComputeShaderViscosity->UnbindProgram();
}

void Fluid::Velocity()
{
    //Bind
    mComputeShaderVelocity->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderVelocity->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderVelocity->GetDeltaTimeLocation(), mDeltaTimePhysics);

    //Bind buffer
    mParticlesBuffer.BindBase(3);

    //Dispatch
    mComputeShaderVelocity->ProcessComputeShader(NB_PARTICLES,1,1);

    //Read processed data
    mParticlesBuffer.ReadFromBuffer(0, &mParticleComputableData[0], NB_PARTICLES*sizeof(ParticleComputableData));

    //Unbind
    mComputeShaderVelocity->UnbindProgram();
}

void Fluid::Collisions()
{
    //Bind
    mComputeShaderCollisions->BindProgram();

    //Uniforms
    mContext.glUniform1ui(mComputeShaderCollisions->GetGridSizeLocation(), mGrid->GetSize());
    mContext.glUniform1ui(mComputeShaderCollisions->GetNbParticuleLocation(), mParticleComputableData.size());
    mContext.glUniform1f(mComputeShaderVelocity->GetDeltaTimeLocation(), mDeltaTimePhysics);
    mContext.glUniform1ui(mComputeShaderCollisions->GetNbCubeCollidersLocation(), PhysicManager::Instance()->GetCubeCollidersData().size());
    for(ushort i = 0; i<3; ++i)
    {
        mContext.glUniform1f(mComputeShaderCollisions->GetStepLocation(i), mGrid->GetStep(i));
        mContext.glUniform1f(mComputeShaderCollisions->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Bind buffer
    mGridBuffer.BindBase(0);
    mVoxelsIndicesCollidersBuffer.BindBase(2);
    mParticlesBuffer.BindBase(3);
    mCollidersBuffer.BindBase(4);

    //Dispatch
    mComputeShaderCollisions->ProcessComputeShader(NB_PARTICLES,1,1);

//    QVector<CubeCollider*> _colliders = PhysicManager::Instance()->GetCubeColliders();
//    for(CubeCollider* _collider : _colliders)
//        if(_collider->GetVelocity()!=QVector3D(0,0,0)) {
//            qDebug() <<"u";
//            _collider->ResetVelocity();
//        }

    //Unbind
    mComputeShaderCollisions->UnbindProgram();
}

void Fluid::GenerateGrid()
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

    QVector3D _bb = QVector3D(_bbX, _bbY, _bbZ);
    QVector3D _BB = QVector3D(_BBX, _BBY, _BBZ);

    mGrid->GenerateBoundingBoxGrid(_bb, _BB);
    //mMarchingCubeGrid->GenerateBoundingBoxGrid(_bb, _BB);
}

void Fluid::RefreshGrid()
{
    //Get physics cube colliders
    QVector<CubeCollider> _cubeColliders = PhysicManager::Instance()->GetCubeCollidersData();

    //Use compute shader
    mComputeShaderGrid->BindProgram();

    //Send uniforms
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniform1ui(mComputeShaderGrid->GetNbParticuleLocation(), mParticleComputableData.size());
    _functions.glUniform1ui(mComputeShaderGrid->GetNbCubeCollidersLocation(), _cubeColliders.size());
    _functions.glUniform1ui(mComputeShaderGrid->GetGridSizeLocation(), mGrid->GetSize());
    for(ushort i = 0; i<3; ++i)
    {
        _functions.glUniform1f(mComputeShaderGrid->GetStepLocation(i), mGrid->GetStep(i));
        _functions.glUniform1f(mComputeShaderGrid->GetbbLocation(i), mGrid->Getbb(i));
    }

    //Copy data and bind buffers
    mGridBuffer.CopyDataToBuffer(mGrid->GetAllVoxels());
    mGridBuffer.BindBase(0);
    mVoxelsIndicesParticlesBuffer.BindBase(1);
    mVoxelsIndicesCollidersBuffer.BindBase(2);
    mParticlesBuffer.BindBase(3);
    mCollidersBuffer.BindBase(4);

    mBufferPresenceParticleInVoxels.BindBase(10);

    //Use CS
    mComputeShaderGrid->ProcessComputeShader(mGrid->GetSize(), mGrid->GetSize(), mGrid->GetSize());

    //Retrieve data from buffers
    mGridBuffer.ReadFromBuffer(0, &mGrid->GetAllVoxels()[0], mGrid->GetNbVoxels()*sizeof(Voxel));
    mVoxelsIndicesParticlesBuffer.ReadFromBuffer(0, &mGrid->GetAllVoxelsParticlesIndices()[0], mGrid->GetNbVoxels()*MAX_PARTICLES_PER_VOXEL*sizeof(uint));
    mVoxelsIndicesCollidersBuffer.ReadFromBuffer(0, &mGrid->GetAllVoxelsCubeCollidersIndices()[0], mGrid->GetNbVoxels()*MAX_CUBE_COLLIDERS_PER_VOXEL*sizeof(uint));

    //Shader
    mComputeShaderGrid->UnbindProgram();
}

void Fluid::MarchingCube()
{
    //Use compute shader
    mComputeShaderMarchingCube->BindProgram();

    //Send uniforms
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniform1ui(mComputeShaderMarchingCube->GetNbParticuleLocation(), mParticleComputableData.size());
    _functions.glUniform1ui(mComputeShaderMarchingCube->GetGridSizeLocation(), mGrid->GetSize());
    _functions.glUniform1ui(mComputeShaderMarchingCube->GetMCGridSizeLocation(), mMarchingCubeGrid->GetSize());
    _functions.glUniform1f(mComputeShaderMarchingCube->GetSeuilLocation(), mMarchingCubeGrid->GetSeuil());
    for(ushort i = 0; i<3; ++i)
    {
        _functions.glUniform1f(mComputeShaderMarchingCube->GetStepLocation(i), mGrid->GetStep(i));
        _functions.glUniform1f(mComputeShaderMarchingCube->GetbbLocation(i), mGrid->Getbb(i));
        _functions.glUniform1f(mComputeShaderMarchingCube->GetStepMCLocation(i), mMarchingCubeGrid->GetStep(i));
        _functions.glUniform1f(mComputeShaderMarchingCube->GetbbMCLocation(i), mMarchingCubeGrid->Getbb(i));
    }

    //Copy data and bind buffers
    mGridBuffer.CopyDataToBuffer(mGrid->GetAllVoxels());
    mGridBuffer.BindBase(0);
    mVoxelsIndicesParticlesBuffer.BindBase(1);
    mMarchingCubeGridBuffer.CopyDataToBuffer(mMarchingCubeGrid->GetAllVoxels());
    mMarchingCubeGridBuffer.BindBase(2);
    mParticlesBuffer.BindBase(3);

    mMCTable.BindBase(10);
    mMCEdgeTable.BindBase(11);

    //Use CS
    mComputeShaderMarchingCube->ProcessComputeShader(mMarchingCubeGrid->GetSize(), mMarchingCubeGrid->GetSize(), mMarchingCubeGrid->GetSize());

    //Retrieve data from buffers
    mMarchingCubeGridBuffer.ReadFromBuffer(0, &mMarchingCubeGrid->GetAllVoxels()[0], mMarchingCubeGrid->GetNbVoxels()*sizeof(MCVoxel));

    //Shader
    mComputeShaderMarchingCube->UnbindProgram();
}

void Fluid::MergeTriangles()
{
    QVector<QVector3D> _positions = QVector<QVector3D>();

    QVector<MCVoxel> _voxels = mMarchingCubeGrid->GetAllVoxels();
    for(const MCVoxel& _voxel : _voxels)
    {
        uint _nbTriangleInVoxel = _voxel.mNbTriangles;
        for(uint _triangleIndex = 0; _triangleIndex < _nbTriangleInVoxel; ++_triangleIndex)
            for(uint _vertexIndex = 0; _vertexIndex < 3; ++_vertexIndex)
            {
                QVector3D _vertex = QVector3D(_voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3], _voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3 +1] , _voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3 +2]);
                _positions.push_back(_vertex);
            }
    }
    QVector<QVector3D> _normales = QVector<QVector3D>(_positions.size(), QVector3D(0,1,0));
    mFluidMesh->RefreshMesh(_positions, _normales, QVector<uint>());
}

void Fluid::LoadShader()
{
    mFluidShader = new ShaderProgram("src/shaders/Vertex/particle_vertex_shader.glsl", "src/shaders/Fragments/particle_fragment_shader.glsl");
    mComputeShaderGravity = new ShaderProgram("src/shaders/Computes/compute_shader_gravity.glsl");
    mComputeShaderDensityPressure = new ShaderProgram("src/shaders/Computes/compute_shader_density_pressure.glsl");
    mComputeShaderGradient = new ShaderProgram("src/shaders/Computes/compute_shader_gradient.glsl");
    mComputeShaderViscosity = new ShaderProgram("src/shaders/Computes/compute_shader_viscosity.glsl");
    mComputeShaderVelocity = new ShaderProgram("src/shaders/Computes/compute_shader_velocity.glsl");
    mComputeShaderCollisions = new ShaderProgram("src/shaders/Computes/compute_shader_collisions.glsl");
    mComputeShaderGrid = new ShaderProgram("src/shaders/Computes/compute_shader_grid.glsl");
    mComputeShaderMarchingCube = new ShaderProgram("src/shaders/Computes/compute_shader_marching_cube.glsl");
}

void Fluid::ComputeCenter()
{
    mCenter = QVector3D(0,0,0);
    for(const ParticleComputableData& _particleData : mParticleComputableData)
        mCenter += QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ);
    mCenter /= (float)mParticleComputableData.size();
}
