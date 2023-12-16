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

void ApplyGravityOnVelocity()
{
    uint index = GetParticleIndex();
    particles[index].velocityY -= 9.8 * deltaTime;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    ApplyGravityOnVelocity();

    uint index = GetParticleIndex();
    particles[index].predictedPositionX = particles[index].positionX + particles[index].velocityX * 1/120.f;
    particles[index].predictedPositionY = particles[index].positionY + particles[index].velocityY * 1/120.f;
    particles[index].predictedPositionZ = particles[index].positionZ + particles[index].velocityZ * 1/120.f;
}
