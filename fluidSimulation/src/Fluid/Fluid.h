#ifndef FLUID_H
#define FLUID_H

#include "../Particle/Particle.h"
#include "../Buffer/Buffer.h"
#include "../ShaderProgram/ShaderProgram.h"

class QGLViewer;

class Fluid
{
    //Particles
    unsigned long mNbParticles = 0;
    QVector<ParticleComputableData> mParticleComputableData;

    //Display
    ShaderProgram* mRenderShader = nullptr;
    Particle* mParticleTemplateDisplay = nullptr;
    bool mDisplayParticles = true;
    float mScaleParticle = 1;
    QVector3D mCenter = QVector3D();

    //Compute
    ShaderProgram* mComputeShader = nullptr;
    OGL_Buffer mParticlesBuffer = OGL_Buffer(QOpenGLBuffer::VertexBuffer);

public:
    Fluid(unsigned long _nbParticles);
    ~Fluid();

public:
    void ProcessFluid();
    void RenderFluid(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;
    QVector3D Center() const {return mCenter;}

private:
    void LoadShader();
    void ComputeCenter();
};

#endif // FLUID_H
