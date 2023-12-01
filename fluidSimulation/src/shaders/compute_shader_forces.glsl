#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
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

uniform float deltaTime = 0.001;

void ApplyGravity()
{
    uint index = gl_GlobalInvocationID.x;
    particles[index].velocityY -= 9.8 * deltaTime;
    particles[index].positionY += particles[index].velocityY * deltaTime;
}

void main()
{
    //ApplyGravity();
}
