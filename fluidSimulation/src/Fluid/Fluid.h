#ifndef FLUID_H
#define FLUID_H

#include "../Particle/Particle.h"
#include "../Buffer/Buffer.h"
#include "../ShaderProgram/ShaderProgram.h"
#include "../Grid/Grid.h"

class QGLViewer;
class CubeCollider;

class Fluid
{
    //Particles
    unsigned int mNbParticles = NB_PARTICLES;
    QVector<ParticleComputableData> mParticleComputableData;

    //Physic
    int mFPSFluid = 30;
    float mTimer = 0;

    //Display
    ShaderProgram* mFluidShader = nullptr;
    Particle* mParticleTemplateDisplay = nullptr;
    bool mDisplayParticles = true;
    float mScaleParticle = 1;
    QVector3D mCenter = QVector3D();

    //Compute Fluid
    ShaderProgram* mComputeShaderForces = nullptr;
    OGL_Buffer mParticlesBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

    //Compute Grid
    Grid* mGrid = nullptr;
    ShaderProgram* mComputeShaderGrid = nullptr;
    OGL_Buffer mVoxelsBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mVoxelsIndicesParticlesBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mCollidersBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);
    OGL_Buffer mVoxelsIndicesCollidersBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

public:
    const Grid* GetGrid() const {return mGrid;}

public:
    Fluid();
    ~Fluid();

public:
    void Initialize();
    void Collisions();
    bool Detection(const QVector3D& _position, const CubeCollider& _collider);
    float Resolution(ParticleComputableData& _particle, const CubeCollider& _collider);
    void UpdateFluid(float _deltaTime);
    void Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;
    QVector3D Center() const {return QVector3D(0,0,0);} //CAN'T CENTER BECAUSE GRID IS NOT DISPLAYING WHERE IT SHOULD

private:
    void RefreshGrid();
    void ApplyForceOnCS();
    void LoadShader();
    void ComputeCenter();
};

#endif // FLUID_H
