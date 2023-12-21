#version 430

////////////////////////////// GRADIENT SHADER //////////////////////////////

layout (local_size_x = 224, local_size_y = 1, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 10240;
const float M_PI = 3.14159265358979323846;

const float SMOOTHING_RADIUS = .9f;
const float SMOOTHING_RADIUS_P4 = SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS;
const float VOLUME = (M_PI * SMOOTHING_RADIUS_P4) / 6;

const float VISCOSITY_STRENGTH = 1.25f;
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

//    float pressureForceX;
//    float pressureForceY;
//    float pressureForceZ;

//    float viscosityForceX;
//    float viscosityForceY;
//    float viscosityForceZ;
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


float ViscositySmoothingKernel(float _distance)
{
//    if(_distance >= SMOOTHING_RADIUS) return 0;
//    float _difference = SMOOTHING_RADIUS - _distance;
//    return _difference * _difference / VOLUME;

    if(_distance >= SMOOTHING_RADIUS) return 0;
    float _difference = max(SMOOTHING_RADIUS*SMOOTHING_RADIUS - _distance*_distance,0);
    return _difference * _difference *_difference;
}

shared vec3[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z] viscosityForces = vec3[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z](0.f);

void ViscosityForce()
{
//    uint index = GetParticleIndex();
//    particles[index].viscosityForceX = 0;
//    particles[index].viscosityForceY = 0;
//    particles[index].viscosityForceZ = 0;
//    barrier();

//    vec3 _position = vec3(particles[index].predictedPositionX, particles[index].predictedPositionY, particles[index].predictedPositionZ);
//    vec3 _velocity = vec3(particles[index].velocityX, particles[index].velocityY, particles[index].velocityZ);

//    float _mass = 1;

//    uint _threadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
//    uint _particlesPerThread = nbParticles/_threadsInGroup;
//    uint _particlesleft = nbParticles % _threadsInGroup;
//    uint _beginLeftParticles = _particlesPerThread * _threadsInGroup;
//    uint _nbLeft = nbParticles % _threadsInGroup;

//    for(uint _particleIndexInVoxel = 0; _particleIndexInVoxel < _particlesPerThread; ++_particleIndexInVoxel)
//    {
//        uint _particleIndex = _particleIndexInVoxel+gl_LocalInvocationIndex*_particlesPerThread;
//        if(_particleIndex == index) continue;
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = ViscositySmoothingKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        vec3 _otherVelocity = vec3(particles[_particleIndex].velocityX, particles[_particleIndex].velocityY, particles[_particleIndex].velocityZ);
//        viscosityForces[gl_LocalInvocationIndex] += (_otherVelocity - _velocity)*_influence;
//    }

//    if(gl_LocalInvocationIndex < _nbLeft)
//    {
//        uint _particleIndex = _beginLeftParticles + gl_LocalInvocationIndex;
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = ViscositySmoothingKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        vec3 _otherVelocity = vec3(particles[_particleIndex].velocityX, particles[_particleIndex].velocityY, particles[_particleIndex].velocityZ);
//        viscosityForces[gl_LocalInvocationIndex] += (_otherVelocity - _velocity)*_influence;
//    }

//    barrier();
//    if(gl_LocalInvocationIndex != 0) return;

//    for(uint i = 0; i < _threadsInGroup; ++i)
//    {
//        particles[index].viscosityForceX += viscosityForces[i].x;
//        particles[index].viscosityForceY += viscosityForces[i].y;
//        particles[index].viscosityForceZ += viscosityForces[i].z;
//    }

//    particles[index].viscosityForceX *= VISCOSITY_STRENGTH;
//    particles[index].viscosityForceY *= VISCOSITY_STRENGTH;
//    particles[index].viscosityForceZ *= VISCOSITY_STRENGTH;

//    particles[index].velocityX += particles[index].viscosityForceX * deltaTime;
//    particles[index].velocityY += particles[index].viscosityForceY * deltaTime;
//    particles[index].velocityZ += particles[index].viscosityForceZ * deltaTime;

    ////////////////////////////////////////////////////////////////////////////////////////

    uint index = GetParticleIndex();
//    particles[index].pressureForceX = 0;
//    particles[index].pressureForceY = 0;
//    particles[index].pressureForceZ = 0;
//    barrier();

    Particle _currentParticle = particles[index];
    vec3 _position = vec3(_currentParticle.predictedPositionX, _currentParticle.predictedPositionY, _currentParticle.predictedPositionZ);
    vec3 _velocity = vec3(_currentParticle.velocityX, _currentParticle.velocityY, _currentParticle.velocityZ);

    //Thread in the group
    uint _nbThreadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;

    //Get grid coord
    vec3 _gridPosition = XYZCoordToGridCoord(_position);

    //Get voxel in range
    int _nbVoxelX = int(ceil(SMOOTHING_RADIUS/stepX));
    int _nbVoxelY = int(ceil(SMOOTHING_RADIUS/stepY));
    int _nbVoxelZ = int(ceil(SMOOTHING_RADIUS/stepZ));

    //Get range from current voxel
    uint _minK = uint(max(_gridPosition.z - _nbVoxelZ, 0.f));
    uint _minJ = uint(max(_gridPosition.y - _nbVoxelY, 0.f));
    uint _minI = uint(max(_gridPosition.x - _nbVoxelX, 0.f));

    uint _maxK = uint(min(_gridPosition.z + _nbVoxelZ, sizeGrid - 1.f));
    uint _maxJ = uint(min(_gridPosition.y + _nbVoxelY, sizeGrid - 1.f));
    uint _maxI = uint(min(_gridPosition.x + _nbVoxelX, sizeGrid - 1.f));

    vec3 _viscosityForce = vec3(0,0,0);

    //For each close voxel
    for(uint _k = _minK; _k <= _maxK; ++_k)
        for(uint _j = _minJ; _j <= _maxJ; ++_j)
            for(uint _i = _minI; _i <= _maxI; ++_i)
            {
                uint _voxelIndex = GridCoordToLinearCoord(_i, _j, _k);
                uint _nbParticlesInVoxel = voxels[_voxelIndex].mNbParticles;
                uint _nbParticlesPerThread = _nbParticlesInVoxel / _nbThreadsInGroup;
                uint _nbParticlesLeft = _nbParticlesInVoxel % _nbThreadsInGroup;

                for(uint _particleIndex = 0; _particleIndex < _nbParticlesPerThread; ++_particleIndex)
                {
                    uint _offsetThreadIndex = _particleIndex + gl_LocalInvocationIndex * _nbParticlesPerThread;
                    uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                    if(_particleIndex == index) continue;
                    Particle _otherParticle = particles[_particleRealIndex];
                    vec3 _otherParticlePosition = vec3(_otherParticle.predictedPositionX, _otherParticle.predictedPositionY, _otherParticle.predictedPositionZ);
                    float _distanceTwoParticles = length(_position - _otherParticlePosition);
                    float _influence = ViscositySmoothingKernel(_distanceTwoParticles);
                    vec3 _otherVelocity = vec3(_otherParticle.velocityX, _otherParticle.velocityY, _otherParticle.velocityZ);
                    _viscosityForce += (_otherVelocity - _velocity)*_influence;
                }

                uint _nbParticlesProcessed = _nbParticlesPerThread * _nbThreadsInGroup;

                if(gl_LocalInvocationIndex < _nbParticlesLeft)
                {
                    uint _offsetThreadIndex = _nbParticlesProcessed + gl_LocalInvocationIndex;
                    uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                    if(_particleRealIndex == index) return;
                    Particle _otherParticle = particles[_particleRealIndex];
                    vec3 _otherParticlePosition = vec3(_otherParticle.predictedPositionX, _otherParticle.predictedPositionY, _otherParticle.predictedPositionZ);
                    float _distanceTwoParticles = length(_position - _otherParticlePosition);
                    float _influence = ViscositySmoothingKernel(_distanceTwoParticles);
                    vec3 _otherVelocity = vec3(_otherParticle.velocityX, _otherParticle.velocityY, _otherParticle.velocityZ);
                    _viscosityForce += (_otherVelocity - _velocity)*_influence;
                }
            }

    viscosityForces[gl_LocalInvocationIndex] = _viscosityForce;

    barrier();
    if(gl_LocalInvocationIndex != 0) return;

    float _viscosityForceX = 0;
    float _viscosityForceY = 0;
    float _viscosityForceZ = 0;
    for(uint i = 0; i < _nbThreadsInGroup; ++i)
    {
        _viscosityForceX += viscosityForces[i].x;
        _viscosityForceY += viscosityForces[i].y;
        _viscosityForceZ += viscosityForces[i].z;
    }

    _viscosityForceX *= VISCOSITY_STRENGTH;
    _viscosityForceY *= VISCOSITY_STRENGTH;
    _viscosityForceZ *= VISCOSITY_STRENGTH;

    particles[index].velocityX += _viscosityForceX * deltaTime;
    particles[index].velocityY += _viscosityForceY * deltaTime;
    particles[index].velocityZ += _viscosityForceZ * deltaTime;

}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    uint index = GetParticleIndex();
    ViscosityForce();
}
