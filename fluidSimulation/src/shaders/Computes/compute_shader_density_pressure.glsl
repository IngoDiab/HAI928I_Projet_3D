#version 430

////////////////////////////// DENSITY/PRESSURE SHADER //////////////////////////////

layout (local_size_x = 224, local_size_y = 1, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 10240;
const float M_PI = 3.14159265358979323846;

const uint NB_THREADS_IN_GROUP = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;

const float SMOOTHING_RADIUS = .6f;
const float SMOOTHING_RADIUS_P4 = SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS;
const float VOLUME = (M_PI * SMOOTHING_RADIUS_P4) / 6;

const float TARGETED_DENSITY = 7.5;
const float PRESSURE_MULTIPLIER = 100;
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

float SmoothKernel(float _distance)
{
    if(_distance >= SMOOTHING_RADIUS) return 0;
    float _difference = SMOOTHING_RADIUS - _distance;
    return _difference * _difference / VOLUME;
}

shared float[NB_THREADS_IN_GROUP] densities = float[NB_THREADS_IN_GROUP](0.f);

float DensityToPressure(float _density)
{
    float _densityError = _density - TARGETED_DENSITY;
    float _pressure = _densityError * PRESSURE_MULTIPLIER;
    return _pressure;
}

void DensityParticle()
{
    uint index = GetParticleIndex();
    //if(index >= nbParticles) return;
    particles[index].density = 0;
    particles[index].pressure = 0;
    barrier();

    Particle _currentParticle = particles[index];

    vec3 _position = vec3(_currentParticle.predictedPositionX, _currentParticle.predictedPositionY, _currentParticle.predictedPositionZ);

    /////////////////////////////////////////////

//    uint _threadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
//    uint _particlesPerThread = nbParticles/_threadsInGroup;
//    uint _particlesleft = nbParticles % _threadsInGroup;
//    uint _beginLeftParticles = _particlesPerThread * _threadsInGroup;
//    uint _nbLeft = nbParticles % _threadsInGroup;

//    for(uint _particleIndexInVoxel = 0; _particleIndexInVoxel < _particlesPerThread; ++_particleIndexInVoxel)
//    {
//        uint _particleIndex = _particleIndexInVoxel+gl_LocalInvocationIndex*_particlesPerThread;
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        densities[gl_LocalInvocationIndex] += _mass * _influence;
//    }

//    if(gl_LocalInvocationIndex < _nbLeft)
//    {
//        uint _particleIndex = _beginLeftParticles + gl_LocalInvocationIndex;
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        densities[gl_LocalInvocationIndex] += _mass * _influence;
//    }

//    barrier();
//    if(gl_LocalInvocationIndex != 0) return;

//    for(uint i = 0; i < _threadsInGroup; ++i)
//        particles[index].density += densities[i];

//    particles[index].pressure = DensityToPressure(particles[index].density);

    /////////////////////////////////////////////

    //Thread in the group
    //uint _nbThreadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;

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

    float _density = 0;

    //For each close voxel
    for(uint _k = _minK; _k <= _maxK; ++_k)
        for(uint _j = _minJ; _j <= _maxJ; ++_j)
            for(uint _i = _minI; _i <= _maxI; ++_i)
            {
                uint _voxelIndex = GridCoordToLinearCoord(_i, _j, _k);
                uint _nbParticlesInVoxel = voxels[_voxelIndex].mNbParticles;
                uint _nbParticlesPerThread = _nbParticlesInVoxel / NB_THREADS_IN_GROUP;
                uint _nbParticlesLeft = _nbParticlesInVoxel % NB_THREADS_IN_GROUP;

                for(uint _particleIndex = 0; _particleIndex < _nbParticlesPerThread; ++_particleIndex)
                {
                    uint _offsetThreadIndex = _particleIndex + gl_LocalInvocationIndex * _nbParticlesPerThread;
                    uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                    Particle _otherParticle = particles[_particleRealIndex];
                    vec3 _otherParticlePosition = vec3(_otherParticle.positionX, _otherParticle.positionY, _otherParticle.positionZ);
                    float _distanceTwoParticles = length(_position - _otherParticlePosition);
                    float _influence = SmoothKernel(_distanceTwoParticles);
                    _density += _influence;
                }

                uint _nbParticlesProcessed = _nbParticlesPerThread * NB_THREADS_IN_GROUP;

                if(gl_LocalInvocationIndex < _nbParticlesLeft)
                {
                    uint _offsetThreadIndex = _nbParticlesProcessed + gl_LocalInvocationIndex;
                    uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                    Particle _otherParticle = particles[_particleRealIndex];
                    vec3 _otherParticlePosition = vec3(_otherParticle.positionX, _otherParticle.positionY, _otherParticle.positionZ);
                    float _distanceTwoParticles = length(_position - _otherParticlePosition);
                    float _influence = SmoothKernel(_distanceTwoParticles);
                    _density += _influence;
                }
            }

    densities[gl_LocalInvocationIndex] = _density;

    barrier();
    if(gl_LocalInvocationIndex != 0) return;

    for(uint i = 0; i < NB_THREADS_IN_GROUP; ++i)
        particles[index].density += densities[i];

    particles[index].pressure = DensityToPressure(particles[index].density);
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    DensityParticle();
}
