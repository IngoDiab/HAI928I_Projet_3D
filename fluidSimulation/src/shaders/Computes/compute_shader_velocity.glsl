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
    Particle _particle = particles[index];

    //Save position as previous
    particles[index].previousPositionX = _particle.positionX;
    particles[index].previousPositionY = _particle.positionY;
    particles[index].previousPositionZ = _particle.positionZ;

    //Apply velocity
    particles[index].positionX += _particle.velocityX * deltaTime;
    particles[index].positionY += _particle.velocityY * deltaTime;
    particles[index].positionZ += _particle.velocityZ * deltaTime;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    ApplyVelocityOnPosition();
}
