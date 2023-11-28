#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct ParticleData
{
    vec3 velocity;
    vec3 position;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    ParticleData particles[5];
};

uniform float deltaTime = 0.01; // Ajustez selon vos besoins

void main() {
    uint index = gl_GlobalInvocationID.x;
    vec3 gravity = vec3(0.0, -9.8, 0.0);
    particles[index].velocity += gravity * deltaTime;
    particles[index].position += particles[index].velocity * deltaTime;
}
