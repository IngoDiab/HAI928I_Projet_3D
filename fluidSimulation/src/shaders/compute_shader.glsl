#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct ParticleData
{
    float velocityX;
    float velocityY;
    float velocityZ;

    float positionX;
    float positionY;
    float positionZ;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    ParticleData particles[];
};

uniform float deltaTime = 0.001;

void ApplyGravity()
{
    uint index = gl_GlobalInvocationID.x;
    particles[index].velocityY -= 9.8 * deltaTime;
    particles[index].positionY += particles[index].velocityY * deltaTime;
}

void FindBoundariesOctree()
{

}

void main()
{
    FindBoundariesOctree();
    ApplyGravity();
}
