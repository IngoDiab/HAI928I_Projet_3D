#ifndef FLUID_H
#define FLUID_H

#include "../Particle/Particle.h"
#include "../Buffer/Buffer.h"
#include "../ShaderProgram/ShaderProgram.h"
#include "../Grid/Grid.h"

class QGLViewer;

class Fluid
{
    //Particles
    unsigned int mNbParticles = 0;
    QVector<ParticleComputableData> mParticleComputableData;

    //Display
    ShaderProgram* mRenderShader = nullptr;
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

public:
    Fluid(unsigned int _nbParticles);
    ~Fluid();

public:
    void RefreshGrid(unsigned int _maxWorkGroupX, unsigned short _maxWorkGroupY, unsigned short _maxWorkGroupZ);
    void ApplyForceOnCS(unsigned int _maxWorkGroupX, unsigned short _maxWorkGroupY, unsigned short _maxWorkGroupZ);
    void Render(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;
    QVector3D Center() const {return QVector3D(0,0,0);} //CAN'T CENTER BECAUSE GRID IS NOT DISPLAYING WHERE IT SHOULD

private:
    void LoadShader();
    void ComputeCenter();
};

#endif // FLUID_H
