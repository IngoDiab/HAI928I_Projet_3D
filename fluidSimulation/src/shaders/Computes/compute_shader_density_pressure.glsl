#version 430

////////////////////////////// DENSITY/PRESSURE SHADER //////////////////////////////

layout (local_size_x = 8, local_size_y = 4, local_size_z = 4) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 10000;
const float M_PI = 3.14159265358979323846;

const uint MAX_VOXELS = 216;

const float SMOOTHING_RADIUS = .3f;
const float TARGETED_DENSITY = 2;
const float PRESSURE_MULTIPLIER = 300;
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

float SmoothKernel(float _radius, float _distance)
{
    if(_distance >= _radius) return 0;
    float _volume = (M_PI * pow(_radius, 4)) / 6;
    return (_radius - _distance) * (_radius - _distance) / _volume;
}

shared float[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z] densities = float[gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z](0.f);

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
        //if(_particleIndex == index) continue;
        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
        float _distanceTwoParticles = length(_position - _otherParticlePosition);
        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
        densities[gl_LocalInvocationIndex] += _mass * _influence;
    }

    if(gl_LocalInvocationIndex < _nbLeft)
    {
        uint _particleIndex = _beginLeftParticles + gl_LocalInvocationIndex;
        //if(_particleIndex == index) return;
        vec3 _otherParticlePosition = vec3(particles[_particleIndex].predictedPositionX, particles[_particleIndex].predictedPositionY, particles[_particleIndex].predictedPositionZ);
        float _distanceTwoParticles = length(_position - _otherParticlePosition);
        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
        densities[gl_LocalInvocationIndex] += _mass * _influence;
    }

    barrier();
    if(gl_LocalInvocationIndex != 0) return;

    for(uint i = 0; i < _threadsInGroup; ++i)
        particles[index].density += densities[i];

    particles[index].pressure = DensityToPressure(particles[index].density);

    /////////////////////////////////////////////

    //Get grid coord
//    vec3 _gridPosition = XYZCoordToGridCoord(_position);

//    //Get voxel in range
//    int _nbVoxelX = int(min(float(sizeGrid/2),float(ceil(SMOOTHING_RADIUS/stepX))));
//    int _nbVoxelY = int(min(float(sizeGrid/2),float(ceil(SMOOTHING_RADIUS/stepY))));
//    int _nbVoxelZ = int(min(float(sizeGrid/2),float(ceil(SMOOTHING_RADIUS/stepZ))));
//    int _nbVoxel = (2*_nbVoxelX+1) * (2*_nbVoxelY+1) * (2*_nbVoxelZ+1);
//    int _nbVoxelsFace = (2*_nbVoxelX+1) * (2*_nbVoxelY+1);

//    //Thread in the group
//    uint _threadsInGroup = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
//    //Voxel linked for this thread
//    uint _voxelIndex = gl_LocalInvocationIndex % _nbVoxel;
//    //How many threads for each voxel
//    uint _threadPerVoxel = _threadsInGroup/_nbVoxel;
//    //Offset thread for particles
//    uint _indexThreadforParticles = gl_LocalInvocationIndex/_nbVoxel;

//    //Get range from current voxel
//    uint _minK = uint(max(_gridPosition.z - _nbVoxelZ, 0.f));
//    uint _minJ = uint(max(_gridPosition.y - _nbVoxelY, 0.f));
//    uint _minI = uint(max(_gridPosition.x - _nbVoxelX, 0.f));

//    uint _maxK = uint(min(_gridPosition.z + _nbVoxelZ, sizeGrid - 1.f));
//    uint _maxJ = uint(min(_gridPosition.y + _nbVoxelY, sizeGrid - 1.f));
//    uint _maxI = uint(min(_gridPosition.x + _nbVoxelX, sizeGrid - 1.f));

//    uint _voxX = _minI + _voxelIndex%(2*_nbVoxelX+1);
//    uint _voxY = _minJ + (_voxelIndex/(2*_nbVoxelX+1)) % (2*_nbVoxelY+1);
//    uint _voxZ = _minK + _voxelIndex/_nbVoxelsFace;
//    uint _indexVoxel = GridCoordToLinearCoord(_voxX, _voxY, _voxZ);

//    //Get all near voxels
////    uint[MAX_VOXELS] _voxelsNear;
////    uint _indexStock = 0;
////    for(uint _k = _minK; _k <= _maxK; ++_k)
////        for(uint _j = _minJ; _j <= _maxJ; ++_j)
////            for(uint _i = _minI; _i <= _maxI; ++_i)
////                _voxelsNear[_indexStock++] = GridCoordToLinearCoord(_i, _j, _k);

//    //How many parrticles for this thread's voxel
//    uint _nbParticles = voxels[_indexVoxel].mNbParticles;
//    //How many particles for each thread
//    uint _particlesPerThread = _nbParticles/_threadPerVoxel;
//    //How many particles left for each thread's voxel
//    uint _nbLeft = _nbParticles % _threadPerVoxel;

//    //float _density = 0;
//    for(uint _particleIndexInVoxel = 0; _particleIndexInVoxel < _particlesPerThread; ++_particleIndexInVoxel)
//    {
//        uint _particleIndex = indicesParticlesVoxelsBuffer[_indexVoxel*MAX_PARTICLES+_indexThreadforParticles*_particlesPerThread+_particleIndexInVoxel];
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].positionX, particles[_particleIndex].positionY, particles[_particleIndex].positionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        densities[gl_LocalInvocationIndex] += _influence;
//    }

//    if(_indexThreadforParticles < _nbLeft)
//    {
//        uint _particleIndex = _particlesPerThread*_threadPerVoxel + _indexThreadforParticles;
//        vec3 _otherParticlePosition = vec3(particles[_particleIndex].positionX, particles[_particleIndex].positionY, particles[_particleIndex].positionZ);
//        float _distanceTwoParticles = length(_position - _otherParticlePosition);
//        float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//        densities[gl_LocalInvocationIndex] += _influence;
//    }

//    //densities[gl_LocalInvocationIndex] = _density;

//    barrier();
//    if(gl_LocalInvocationIndex != 0) return;

//    for(uint i = 0; i < _threadsInGroup; ++i)
//        particles[index].density += densities[i];

    ////////////////////////////////////////////////////

//    if(gl_LocalInvocationIndex != 0) return;

//    //Get grid coord
//    vec3 _gridPosition = XYZCoordToGridCoord(_position);

//    int _nbVoxelX = int(ceil(SMOOTHING_RADIUS/stepX));
//    int _nbVoxelY = int(ceil(SMOOTHING_RADIUS/stepY));
//    int _nbVoxelZ = int(ceil(SMOOTHING_RADIUS/stepZ));

//    //Get range from current voxel
//    uint _minK = uint(max(_gridPosition.z - _nbVoxelZ, 0.f));
//    uint _minJ = uint(max(_gridPosition.y - _nbVoxelY, 0.f));
//    uint _minI = uint(max(_gridPosition.x - _nbVoxelX, 0.f));

//    uint _maxK = uint(min(_gridPosition.z + _nbVoxelZ, sizeGrid - 1.f));
//    uint _maxJ = uint(min(_gridPosition.y + _nbVoxelY, sizeGrid - 1.f));
//    uint _maxI = uint(min(_gridPosition.x + _nbVoxelX, sizeGrid - 1.f));

//    //For each voxel in range
//    for(uint _k = _minK; _k <= _maxK; ++_k)
//        for(uint _j = _minJ; _j <= _maxJ; ++_j)
//            for(uint _i = _minI; _i <= _maxI; ++_i)
//            {
//                uint _linearCoord = GridCoordToLinearCoord(_i, _j, _k);

//                //Get colliders of this voxel
//                uint _nbParticlesIndexInVoxel = voxels[_linearCoord].mNbParticles;

//                //For each colliders of this voxel
//                for(uint _particleIndexInVoxel = 0; _particleIndexInVoxel < _nbParticlesIndexInVoxel; ++_particleIndexInVoxel)
//                {
//                    //Check if this collider has already been used
//                    uint _particleIndex = indicesParticlesVoxelsBuffer[_linearCoord*MAX_PARTICLES+_particleIndexInVoxel];
//                    vec3 _otherParticlePosition = vec3(particles[_particleIndex].positionX, particles[_particleIndex].positionY, particles[_particleIndex].positionZ);
//                    float _distanceTwoParticles = length(_position - _otherParticlePosition);
//                    float _influence = SmoothKernel(SMOOTHING_RADIUS, _distanceTwoParticles);
//                    particles[index].density += _influence;
//                }
//            }
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    DensityParticle();
}
