#ifndef FLUID_H
#define FLUID_H

#include <vector>
using namespace std;

#include "../Particle/Particle.h"
#include "../ShaderProgram/ShaderProgram.h"

class QGLViewer;

class Fluid
{
    QVector3D mCenter = QVector3D();

    unsigned long mNbParticles = 2;
    float mScaleParticle = 0.1;
    vector<Particle> mParticles;

    //Display
    ShaderProgram* mShader = nullptr;
    bool mDisplayParticles = true;

public:
    Fluid(unsigned long _nbParticles);
    ~Fluid();

public:
    void RenderFluid(const GLfloat* _projectionMatrix, const GLfloat* _viewMatrix) const;
    QVector3D Center() const {return mCenter;}

private:
    void LoadShader();
    void ComputeCenter();
};

#endif // FLUID_H
