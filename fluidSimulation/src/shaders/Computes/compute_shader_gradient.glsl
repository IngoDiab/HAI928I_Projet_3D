#version 430

////////////////////////////// GRADIENT SHADER //////////////////////////////

layout (local_size_x = 8, local_size_y = 4, local_size_z = 4) in;

//////////////////////////////  CONST //////////////////////////////
const float M_PI = 3.14159265358979323846;

const uint MAX_VOXELS = 216;

const float SMOOTHING_RADIUS = .6f;
//////////////////////////////  VOXELS //////////////////////////////
struct Voxel
{
    float mbbX;
    float mbbY;
    float mbbZ;

    float mBBX;
    float mBBY;
    float mBBZ;

    float corners[24];

    uint mNbParticles;
    uint mNbCubeCollider;
};

layout(std430, binding = 0) buffer VoxelBuffer
{
    Voxel voxels[];
};

layout(std430, binding = 1) buffer IndicesParticlesBuffer
{
    uint indicesParticlesVoxelsBuffer[];
};

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

uniform float stepX;
uniform float stepY;
uniform float stepZ;

uniform float bbX;
uniform float bbY;
uniform float bbZ;

uniform uint sizeGrid;
uniform uint nbParticles;
uniform float time;
//////////////////////////////  INDEX //////////////////////////////

uint GetParticleIndex()
{
    return gl_WorkGroupID.x;
}

//////////////////////////////  GRID SEARCH //////////////////////////////

vec3 XYZCoordToGridCoord(vec3 _xyzPosition)
{
    float _distanceX = sizeGrid * stepX;
    float _distanceY = sizeGrid * stepY;
    float _distanceZ = sizeGrid * stepZ;
    uint _i = uint(((_xyzPosition.x-bbX)/_distanceX)*sizeGrid);
    uint _j = uint(((_xyzPosition.y-bbY)/_distanceY)*sizeGrid);
    uint _k = uint(((_xyzPosition.z-bbZ)/_distanceZ)*sizeGrid);
    return vec3(_i, _j, _k);
}

uint GridCoordToLinearCoord(uint _i, uint _j, uint _k)
{
    return _k*sizeGrid*sizeGrid + _j*sizeGrid + _i;
}

//////////////////////////////  FLUID FORCE //////////////////////////////

vec3 RandomDir()
{
    float seed = sin(time);

    float randX = fract(sin(seed * 12.9898) * 43758.5453);
    float randY = fract(sin((seed + 1.0) * 12.9898) * 43758.5453);
    float randZ = fract(sin((seed + 2.0) * 12.9898) * 43758.5453);

    vec3 _dir = vec3(randX, randY, randZ);
    return _dir / length(_dir);
}

float SmoothKernelDerivative(float _radius, float _distance)
{
    if(_distance >= _radius) return 0;
    float _scale = 12 / (pow(_radius, 4)* M_PI);
    return (_distance - _radius) * _scale;
}

shared vec3[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z] pressureForces = vec3[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z](0.f);

void PressureForce()
{
    uint index = GetParticleIndex();
    //if(index >= nbParticles) return;
    particles[index].pressureForceX = 0;
    particles[index].pressureForceY = 0;
    particles[index].pressureForceZ = 0;
    barrier();

    vec3 _position = vec3(particles[index].predictedPositionX, particles[index].predictedPositionY, particles[index].predictedPositionZ);

    float _mass = 1;
    /////////////////////////////////////////////

    uint _threadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    uint _particlesPerThread = nbParticles/_threadsInGroup;
    uint _particlesleft = nbParticles % _threadsInGroup;
    uint _beginLeftParticles = _particlesPerThread * _threadsInGroup;
    uint _nbLeft = nbParticles % _threadsInGroup;

    for(uint _particleIndexInVoxel = 0; _particleIndexInVoxel < _particlesPerThread; ++_particleIndexInVoxel)
    {
        uint _particleIndex = _particleIndexInVoxel+gl_LocalInvocationIndex*_particlesPerThread;
        if(_particleIndex == index) continue;
        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
        float _distanceTwoParticles = length(_position - _otherParticlePosition);
        vec3 _dir = _distanceTwoParticles == 0 ?  RandomDir() : normalize(_otherParticlePosition-_position);
        float _slope = SmoothKernelDerivative(SMOOTHING_RADIUS, _distanceTwoParticles);
        float _sharedPressure = (particles[index].pressure + particles[_particleIndex].pressure)/2.f;
        pressureForces[gl_LocalInvocationIndex] += _sharedPressure * _dir * _mass * _slope / particles[_particleIndex].density;
    }

    if(gl_LocalInvocationIndex < _nbLeft)
    {
        uint _particleIndex = _beginLeftParticles + gl_LocalInvocationIndex;
        if(_particleIndex == index) return;
        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
        float _distanceTwoParticles = length(_position - _otherParticlePosition);
        vec3 _dir = _distanceTwoParticles == 0 ?  RandomDir() : normalize(_otherParticlePosition-_position);
        float _slope = SmoothKernelDerivative(SMOOTHING_RADIUS, _distanceTwoParticles);
        float _sharedPressure = (particles[index].pressure + particles[_particleIndex].pressure)/2.f;
        pressureForces[gl_LocalInvocationIndex] += _sharedPressure * _dir * _mass * _slope / particles[_particleIndex].density;
    }

    barrier();
    if(gl_LocalInvocationIndex != 0) return;
    barrier();

    for(uint i = 0; i < _threadsInGroup; ++i)
    {
        particles[index].pressureForceX += pressureForces[i].x;
        particles[index].pressureForceY += pressureForces[i].y;
        particles[index].pressureForceZ += pressureForces[i].z;
    }
}

void ApplyAccOnVelocity(vec3 _acc)
{
    uint index = GetParticleIndex();
    if(index >= nbParticles || gl_LocalInvocationIndex != 0) return;
    particles[index].velocityX += _acc.x * deltaTime;
    particles[index].velocityY += _acc.y * deltaTime;
    particles[index].velocityZ += _acc.z * deltaTime;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    uint index = GetParticleIndex();
    PressureForce();
    vec3 _pressureAcc = vec3(particles[index].pressureForceX/ particles[index].density, particles[index].pressureForceY/ particles[index].density, particles[index].pressureForceZ/ particles[index].density);
    ApplyAccOnVelocity(_pressureAcc);
}
