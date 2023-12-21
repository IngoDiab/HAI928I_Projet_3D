#include "Fluid.h"
#include "../Mesh/Sphere/Sphere.h"

#include <QGLViewer/qglviewer.h>
#include <QRandomGenerator>

#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../Physics/PhysicManager/PhysicManager.h"
#include "../Obstacle/Obstacle.h"

#include "../Utils/MarchingCube_Configurations.h"

Fluid::Fluid()
{
    mContext = QOpenGLExtraFunctions(QOpenGLContext::currentContext());

    mGrid = new Grid(5);
    mMarchingCubeGrid = new MCGrid(15);

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

    GenerateGrids(QVector<Obstacle*>());
    RefreshGrid();
}

void Fluid::UpdateFluid(float _deltaTime, QVector<Obstacle*> _containers)
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
    GenerateGrids(_containers);
    RefreshGrid();
//    MarchingCube();
//    MergeTriangles();
}

void Fluid::Render(const QVector3D& _cameraPosition, const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const
{
    if (!mDisplayParticles || !mParticleTemplateDisplay) return;

    //Shader
    mFluidShader->BindProgram();

    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());

    //Send camera position
    float _cameraPositionFloats[3] = {_cameraPosition.x(), _cameraPosition.y(), _cameraPosition.z()};
    _functions.glUniform3fv(mFluidShader->GetCameraPositionLocation(), 1, _cameraPositionFloats);

    //View Projection matrix
    mFluidShader->SendVP(_viewMatrix, _projectionMatrix);
    for(int i = 0; i < mParticleComputableData.size(); ++i)
    {
        //if(i%100 !=0) continue;
        const ParticleComputableData& _particleData = mParticleComputableData[i];
        mParticleTemplateDisplay->GetTransform().SetWorldPosition(QVector3D(_particleData.mPositionX, _particleData.mPositionY, _particleData.mPositionZ));
        mParticleTemplateDisplay->SetVelocity(QVector3D(_particleData.mVelocityX, _particleData.mVelocityY, _particleData.mVelocityZ));

        //Model matrix
        GLfloat* _modelMatrixF = mParticleTemplateDisplay->GetTransform().GetModelMatrix().data();
        mFluidShader->SendM(_modelMatrixF);

        //Draw
        MyMesh* _mesh = mParticleTemplateDisplay->GetMesh();
        if(!_mesh) return;
        _mesh->DrawMesh();
    }

    //mFluidMesh->DrawMesh();

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

void Fluid::GenerateGrids(const QVector<Obstacle*>& _containers)
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


    _bbX = numeric_limits<float>::max(), _bbY = numeric_limits<float>::max(), _bbZ = numeric_limits<float>::max();
    _BBX = numeric_limits<float>::lowest(), _BBY = numeric_limits<float>::lowest(), _BBZ = numeric_limits<float>::lowest();

    for(const Obstacle* _obstacle : _containers)
    {
        CubeCollider* _collider = _obstacle->GetCollider();
        for(uint i = 0; i < 8; ++i)
        {
            QVector3D _cornerPos = _collider->GetCorner(i);
            _bbX = min(_bbX, _cornerPos.x());
            _BBX = max(_BBX, _cornerPos.x());

            _bbY = min(_bbY, _cornerPos.y());
            _BBY = max(_BBY, _cornerPos.y());

            _bbZ = min(_bbZ, _cornerPos.z());
            _BBZ = max(_BBZ, _cornerPos.z());
        }
    }

    _bb = QVector3D(_bbX, _bbY, _bbZ);
    _BB = QVector3D(_BBX, _BBY, _BBZ);
    mMarchingCubeGrid->GenerateBoundingBoxGrid(_bb, _BB);
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

int Fluid::SearchVertex(const QVector<QVector3D>& _vertices, const QVector3D& _position)
{
    for(int _vertexIndex = 0; _vertexIndex < _vertices.size(); ++_vertexIndex)
        if((_vertices[_vertexIndex] - _position).length() <= 0.001f) return _vertexIndex;
    return -1;
}

void Fluid::MergeTriangles()
{
    QVector<MCVoxel> _voxels = mMarchingCubeGrid->GetAllVoxels();

    QVector<QVector3D> _vertices = QVector<QVector3D>();
    QVector<QVector3D> _normales = QVector<QVector3D>();
    QVector<uint> _nbNormalesAdded = QVector<uint>();
    QVector<uint> _indices = QVector<uint>();

    for(const MCVoxel& _voxel : _voxels)
    {
        uint _nbTriangleInVoxel = _voxel.mNbTriangles;
        for(uint _triangleIndex = 0; _triangleIndex < _nbTriangleInVoxel; ++_triangleIndex)
        {
            QVector<QVector3D> _triangle = QVector<QVector3D>(3, QVector3D());
            QVector3D _normaleTriangle = QVector3D(_voxel.mTrianglesNormales[_triangleIndex*3], _voxel.mTrianglesNormales[_triangleIndex*3 +1], _voxel.mTrianglesNormales[_triangleIndex*3 +2]);
            for(uint _vertexIndex = 0; _vertexIndex < 3; ++_vertexIndex)
            {
                QVector3D _vertex = QVector3D(_voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3], _voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3 +1] , _voxel.mTrianglesPos[_triangleIndex*9 + _vertexIndex*3 +2]);
                int _indexInVertices = SearchVertex(_vertices, _vertex);
                if(_indexInVertices != -1)
                {
                    _indices.push_back(_indexInVertices);
                    _normales[_indexInVertices] += _normaleTriangle;
                    ++(_nbNormalesAdded[_indexInVertices]);
                }
                else
                {
                    uint _newIndex = _vertices.size();
                    _vertices.push_back(_vertex);
                    _indices.push_back(_newIndex);
                    _normales.push_back(_normaleTriangle);
                    _nbNormalesAdded.push_back(1);
                }
                _triangle[_vertexIndex] = _vertex;
            }
        }
    }

    for(uint _indexNormale = 0; _indexNormale < _normales.size(); ++_indexNormale)
        _normales[_indexNormale] /= (float)_nbNormalesAdded[_indexNormale];

//    QVector<QVector3D> _normales = QVector<QVector3D>(_positions.size(), QVector3D(0,1,0));
    mFluidMesh->RefreshMesh(_vertices, _normales, _indices);
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
