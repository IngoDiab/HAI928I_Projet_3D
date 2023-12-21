#ifndef FLUID_H
#define FLUID_H

#include "../Particle/Particle.h"
#include "../Buffer/Buffer.h"
#include "../ShaderProgram/ShaderProgram.h"
#include "../Grid/Grid.h"
#include "../Grid/MCGrid.h"

#include "../Utils/Macros.h"

class QGLViewer;
class CubeCollider;
class Obstacle;

class Fluid
{
    QOpenGLExtraFunctions mContext;

    //Particles
    unsigned int mNbParticles = NB_PARTICLES;
    QVector<ParticleComputableData> mParticleComputableData;

    //Physic
    int mFPSFluid = 120;
    float mDeltaTimePhysics = 0;
    float mTimer = 0;

    //Display
    ShaderProgram* mFluidShader = nullptr;
    MyMesh* mFluidMesh = nullptr;
    Particle* mParticleTemplateDisplay = nullptr;
    bool mDisplayParticles = true;
    QVector3D mCenter = QVector3D();

    //Compute Fluid
    ShaderProgram* mComputeShaderGravity = nullptr;
    ShaderProgram* mComputeShaderDensityPressure = nullptr;
    ShaderProgram* mComputeShaderGradient = nullptr;
    ShaderProgram* mComputeShaderViscosity = nullptr;
    ShaderProgram* mComputeShaderVelocity = nullptr;
    ShaderProgram* mComputeShaderCollisions = nullptr;
    OGL_Buffer mParticlesBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

    //Compute Grid
    Grid* mGrid = nullptr;
    MCGrid* mMarchingCubeGrid = nullptr;
    ShaderProgram* mComputeShaderGrid = nullptr;
    ShaderProgram* mComputeShaderMarchingCube = nullptr;
    OGL_Buffer mGridBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mMarchingCubeGridBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mVoxelsIndicesParticlesBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mCollidersBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mVoxelsIndicesCollidersBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

    //Only use as big arrays in computesShader
    OGL_Buffer mBufferPresenceParticleInVoxels = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mMCTable = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mMCEdgeTable = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

public:
    const Grid* GetGrid() const {return mGrid;}
    const MCGrid* GetMarchingCubeGrid() const {return mMarchingCubeGrid;}

public:
    Fluid();
    ~Fluid();

public:
    void Initialize();
    void UpdateFluid(float _deltaTime, QVector<Obstacle*> _containers);

    void Gravity();
    void Density_Pressure();
    void Gradient();
    void Viscosity();
    void Velocity();
    void Collisions();
    void RefreshGrid();
    void MarchingCube();
    int SearchVertex(const QVector<QVector3D>& _vertices, const QVector3D& _position);
    void MergeTriangles();

    //bool Detection(const QVector3D& _position, const CubeCollider& _collider);
    //float Resolution(ParticleComputableData& _particle, const CubeCollider& _collider);
    void GenerateGrids(const QVector<Obstacle*>& _containers);
    void Render(const QVector3D& _cameraPosition, const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;

private:
    void ApplyForceOnCS();
    void LoadShader();
    void ComputeCenter();
};

#endif // FLUID_H
