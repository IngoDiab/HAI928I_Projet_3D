#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
    float previousPositionX;
    float previousPositionY;
    float previousPositionZ;

    float positionX;
    float positionY;
    float positionZ;

    float velocityX;
    float velocityY;
    float velocityZ;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    Particle particles[];
};

uniform float deltaTime;

void ApplyGravityOnVelocity()
{
    uint index = gl_GlobalInvocationID.x;
    particles[index].velocityY -= 9.8 * deltaTime;
}

void ApplyVelocityOnPosition()
{
    uint index = gl_GlobalInvocationID.x;

    //Save position as previous
    particles[index].previousPositionX = particles[index].positionX;
    particles[index].previousPositionY = particles[index].positionY;
    particles[index].previousPositionZ = particles[index].positionZ;

    //Apply velocity
    particles[index].positionX += particles[index].velocityX * deltaTime;
    particles[index].positionY += particles[index].velocityY * deltaTime;
    particles[index].positionZ += particles[index].velocityZ * deltaTime;
}

void main()
{
    ApplyGravityOnVelocity();
    ApplyVelocityOnPosition();
}
