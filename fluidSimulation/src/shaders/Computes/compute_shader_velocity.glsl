#version 430

////////////////////////////// VELOCITY SHADER //////////////////////////////

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//////////////////////////////  PARTICLES //////////////////////////////

struct Particle
{
    float previousPositionX;
    float previousPositionY;
    float previousPositionZ;

    float positionX;
    float positionY;
    float positionZ;

    float predictedPositionX;
    float predictedPositionY;
    float predictedPositionZ;

    float velocityX;
    float velocityY;
    float velocityZ;

    float density;
    float pressure;

    float pressureForceX;
    float pressureForceY;
    float pressureForceZ;
};

layout(std430, binding = 3) buffer ParticleBuffer
{
    Particle particles[];
};

//////////////////////////////  UNIFORMS //////////////////////////////
uniform float deltaTime;
uniform uint nbParticles;

//////////////////////////////  INDEX //////////////////////////////

uint GetParticleIndex()
{
    return gl_WorkGroupID.x;
}

//////////////////////////////  GRAVITY //////////////////////////////

void ApplyVelocityOnPosition()
{
    uint index = GetParticleIndex();

    //Save position as previous
    particles[index].previousPositionX = particles[index].positionX;
    particles[index].previousPositionY = particles[index].positionY;
    particles[index].previousPositionZ = particles[index].positionZ;

    //Apply velocity
    particles[index].positionX += particles[index].velocityX * deltaTime;
    particles[index].positionY += particles[index].velocityY * deltaTime;
    particles[index].positionZ += particles[index].velocityZ * deltaTime;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    ApplyVelocityOnPosition();
}
