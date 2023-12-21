#version 430

////////////////////////////// GRAVITY SHADER //////////////////////////////

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

void ApplyGravityOnVelocity()
{
    uint index = GetParticleIndex();
    particles[index].velocityY -= 9.8 * deltaTime;
}

void PredictPosition()
{
    uint index = GetParticleIndex();
    Particle _particle = particles[index];
    particles[index].predictedPositionX = _particle.positionX + _particle.velocityX * deltaTime;
    particles[index].predictedPositionY = _particle.positionY + _particle.velocityY * deltaTime;
    particles[index].predictedPositionZ = _particle.positionZ + _particle.velocityZ * deltaTime;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    ApplyGravityOnVelocity();
    PredictPosition();
}
