#version 430

////////////////////////////// GRID SHADER //////////////////////////////

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 10000;
const int MAX_CUBE_COLLIDERS = 10;

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

layout(std430, binding = 2) buffer IndicesCubeColliderBuffer
{
    uint indicesCubeColliderVoxelsBuffer[];
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

//////////////////////////////  COLLIDERS //////////////////////////////

struct CubeCollider
{
    int placeHolderTransform;
    float corners[24];
};

layout(std430, binding = 4) buffer CubeColliderBuffer
{
    CubeCollider cubeColliders[];
};

vec3 GetCorner(uint _indexCorner, CubeCollider _collider) {return vec3(_collider.corners[_indexCorner*3], _collider.corners[_indexCorner*3+1], _collider.corners[_indexCorner*3+2]);}

vec3 Getbb(CubeCollider _collider) {return GetCorner(0, _collider);}
vec3 GetBB(CubeCollider _collider) {return GetCorner(6, _collider);}

vec3 GetXAxisCollision(CubeCollider _collider) {return normalize(GetCorner(1, _collider) - GetCorner(0, _collider));}
vec3 GetYAxisCollision(CubeCollider _collider) {return normalize(GetCorner(5, _collider) - GetCorner(0, _collider));}
vec3 GetZAxisCollision(CubeCollider _collider) {return normalize(GetCorner(2, _collider) - GetCorner(0, _collider));}

//////////////////////////////  UNIFORMS //////////////////////////////

uniform float stepX;
uniform float stepY;
uniform float stepZ;

uniform float bbX;
uniform float bbY;
uniform float bbZ;

uniform uint sizeGrid;
uniform uint nbParticles;
uniform uint nbCubeColliders;

//////////////////////////////  INDEX //////////////////////////////

uint GetVoxelThreadIndex()
{
    uint indexI = gl_WorkGroupID.x;
    uint indexJ = gl_WorkGroupID.y;
    uint indexK = gl_WorkGroupID.z;
    return indexK * gl_NumWorkGroups.x * gl_NumWorkGroups.y + indexJ * gl_NumWorkGroups.x + indexI;
}

//////////////////////////////  CREATE VOXELS //////////////////////////////

void CreateGrid()
{
    uint _index = GetVoxelThreadIndex();
    uint indexI = gl_WorkGroupID.x;
    uint indexJ = gl_WorkGroupID.y;
    uint indexK = gl_WorkGroupID.z;

    float vbbX = bbX + indexI*stepX;
    voxels[_index].mbbX = vbbX;
    float vBBX = bbX + (indexI+1)*stepX;
    voxels[_index].mBBX = vBBX;

    float vbbY = bbY + indexJ*stepY;
    voxels[_index].mbbY = vbbY;
    float vBBY = bbY + (indexJ+1)*stepY;
    voxels[_index].mBBY = vBBY;

    float vbbZ = bbZ + indexK*stepZ;
    voxels[_index].mbbZ = vbbZ;
    float vBBZ = bbZ + (indexK+1)*stepZ;
    voxels[_index].mBBZ = vBBZ;

    uint _cornerIndex = gl_LocalInvocationIndex;
    if(_cornerIndex>=8)return;
    voxels[_index].corners[3*_cornerIndex] = (_cornerIndex%4 == 0 || _cornerIndex%4 == 1) ? vbbX : vBBX;
    voxels[_index].corners[3*_cornerIndex+1] = (_cornerIndex%2 == 1) ? vbbY : vBBY;
    voxels[_index].corners[3*_cornerIndex+2] = (_cornerIndex/4 == 1) ? vbbZ : vBBZ;

//    for(uint i = 0; i < 24; i+=3)
//    {
//        voxels[_index].corners[i] = ((i%12)%4 == 0 || (i%12)%4 == 3) ? vbbX : vBBX;
//        voxels[_index].corners[i+1] = (((i+1)%12)%2 == 0) ? vbbY : vBBY;
//        voxels[_index].corners[i+2] = ((i+2)/12 == 1) ? vbbZ : vBBZ;
//    }

//    voxels[_index].corners[0] = vec3(vbbX, vBBY, vBBZ);
//    voxels[_index].corners[1] = vec3(vbbX, vbbY, vBBZ);
//    voxels[_index].corners[2] = vec3(vBBX, vBBY, vBBZ);
//    voxels[_index].corners[3] = vec3(vBBX, vbbY, vBBZ);

//    voxels[_index].corners[4] = vec3(vbbX, vBBY, vbbZ);
//    voxels[_index].corners[5] = vec3(vbbX, vbbY, vbbZ);
//    voxels[_index].corners[6] = vec3(vBBX, vBBY, vbbZ);
//    voxels[_index].corners[7] = vec3(vBBX, vbbY, vbbZ);

    //barrier();
}

//////////////////////////////  PROCESS PARTICLES //////////////////////////////

bool IsParticleInVoxel(const Voxel _voxel, const Particle _particle)
{
    bool _inXaxis = _voxel.mbbX <= _particle.positionX && _particle.positionX <= _voxel.mBBX;
    bool _inYaxis = _voxel.mbbY <= _particle.positionY && _particle.positionY <= _voxel.mBBY;
    bool _inZaxis = _voxel.mbbZ <= _particle.positionZ && _particle.positionZ <= _voxel.mBBZ;
    return _inXaxis && _inYaxis && _inZaxis;
}

void FillVoxelWithParticles()
{
    //Get voxel of this group
    uint _index = GetVoxelThreadIndex();
    atomicExchange(voxels[_index].mNbParticles, 0);

    //Get nb threads available
    uint _threadsInGroup= gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;

    //Synchronize all threads usefull to prevent a thread making mNbParticles = 0 if particle has been added by another thread
    barrier();

    //Less particles than threads or equals
    if(nbParticles<=_threadsInGroup)
    {
        //Get thread index and return if useless or particle not in voxel
        uint _particleIndex = gl_LocalInvocationIndex;
        if(_particleIndex >= nbParticles) return;
        if(!IsParticleInVoxel(voxels[_index], particles[_particleIndex])) return;

        //Add each thread particle index and increment mNbParticles
        //atomicExchange(voxels[_index].mAllParticles[voxels[_index].mNbParticles], _particleIndex);
        atomicExchange(indicesParticlesVoxelsBuffer[_index*MAX_PARTICLES+voxels[_index].mNbParticles], _particleIndex);
        atomicAdd(voxels[_index].mNbParticles,1);
    }

    //More particle than thread (multiple of threads)
    else
    {
        //Get nb particles by thread available
        uint _particlesPerThread = nbParticles/_threadsInGroup;
        //Get thread offset in all particles' table
        uint _threadOffset = _particlesPerThread*gl_LocalInvocationIndex;

        //For each particle per thread
        for(uint i = 0; i < _particlesPerThread; ++i)
        {
            //Return if useless or particle not in voxel
            if(i+_threadOffset >= nbParticles) return;
            if(!IsParticleInVoxel(voxels[_index], particles[i+_threadOffset])) continue;

            //Add each thread particle index and increment mNbParticles
            //atomicExchange(voxels[_index].mAllParticles[voxels[_index].mNbParticles], i+_threadOffset);
            atomicExchange(indicesParticlesVoxelsBuffer[_index*MAX_PARTICLES+voxels[_index].mNbParticles], i+_threadOffset);
            atomicAdd(voxels[_index].mNbParticles,1);
        }
    }

//    for(uint i = 0; i < nbParticles; ++i)
//    {
//        if(!IsParticleInVoxel(voxels[_index], particles[i])) continue;
//       indicesParticlesVoxelsBuffer[_index*MAX_PARTICLES+voxels[_index].mNbParticles] = i;
//        if(voxels[_index].mNbParticles >= MAX_PARTICLES) return;
//    }

    //Synchronize all threads because end of function
    //barrier();
}

//////////////////////////////  PROCESS COLLIDERS //////////////////////////////

shared int[6*MAX_CUBE_COLLIDERS] min_max_projection = int[6*MAX_CUBE_COLLIDERS](0);

bool IsCubeColliderInVoxel(const Voxel _voxel, const CubeCollider _collider)
{
    vec3 _colliderbb = Getbb(_collider);
    vec3 _colliderBB = GetBB(_collider);

    vec3 _voxelbb = vec3(_voxel.mbbX, _voxel.mbbY, _voxel.mbbZ);
    vec3 _voxelBB = vec3(_voxel.mBBX, _voxel.mBBY, _voxel.mBBZ);

    //Get relatives axis cube
    vec3 _axis[3];
    _axis[0] = GetXAxisCollision(_collider);
    _axis[1] = GetYAxisCollision(_collider);
    _axis[2] = GetZAxisCollision(_collider);

    //Each thread has an axis and a corner to test, if no axis the thread is useless
    uint _indexThreadAxis = gl_LocalInvocationID.y/8;
    if(_indexThreadAxis >= 3) return false;
    uint _indexThreadCorner = gl_LocalInvocationID.y%8;

    //Get axis and corner of the thread
    vec3 _currentAxis = _axis[_indexThreadAxis];
    vec3 _vCorner = vec3(_voxel.corners[_indexThreadCorner*3], _voxel.corners[_indexThreadCorner*3+1], _voxel.corners[_indexThreadCorner*3+2]);

    //Project cubeCollider
    float _cbbProjDistancebb = dot(_colliderbb -_colliderbb, _currentAxis);
    float _cBBProjDistancebb = dot(_colliderBB -_colliderbb, _currentAxis);

    //Project corner
    float _vCornerOnProjAxis = dot(_vCorner -_colliderbb, _currentAxis);

    //If the projection is above min or under max, store that it exists for this collider
    uint _cubeColliderIndex = gl_LocalInvocationID.x;
    if(_vCornerOnProjAxis <= _cBBProjDistancebb) atomicOr(min_max_projection[6*_cubeColliderIndex + 2*_indexThreadAxis], 1);
    if(_vCornerOnProjAxis >= _cbbProjDistancebb) atomicOr(min_max_projection[6*_cubeColliderIndex + 2*_indexThreadAxis+1], 1);

    //Synchronize threads after projecting their corner on their axis and storing it
    barrier();

    //Use only one thread
    if(gl_LocalInvocationID.y != 0) return false;

    //Only first thread of this column return the true value;
    return bool(min_max_projection[6*_cubeColliderIndex]) && bool(min_max_projection[6*_cubeColliderIndex+1])
           && bool(min_max_projection[6*_cubeColliderIndex+2]) && bool(min_max_projection[6*_cubeColliderIndex+3])
           && bool(min_max_projection[6*_cubeColliderIndex+4]) && bool(min_max_projection[6*_cubeColliderIndex+5]);

//    bool _isColliding = true;
//    for(uint i = 0; i < 3; ++i)
//    {
//        vec3 _currentAxis = _axis[i];

//        //Project cubeCollider
//        float _cCenterProjDistancebb = dot((_colliderbb + _colliderBB/2.0f) -_colliderbb, _currentAxis);
//        vec3 _cCenterProjPos = _colliderbb + _cCenterProjDistancebb * _currentAxis;
//        float _cbbProjDistancebb = dot(_colliderbb -_colliderbb, _currentAxis);
//        vec3 _cbbProjPos = _colliderbb + _cbbProjDistancebb * _currentAxis;
//        float _cBBProjDistancebb = dot(_colliderBB -_colliderbb, _currentAxis);
//        vec3 _cBBProjPos = _colliderbb + _cBBProjDistancebb * _currentAxis;

//        //Project voxel center
//        float _vCenterProjDistancebb = dot((_voxelbb + _voxelBB/2.0f ) -_colliderbb, _currentAxis);
//        vec3 _vCenterProjPos = _colliderbb + _vCenterProjDistancebb * _currentAxis;

//        float _vMinCoordOnProjAxis = _vCenterProjDistancebb, _vMaxCoordOnProjAxis = _vCenterProjDistancebb;
//        vec3 _vMinProjPos = _vCenterProjPos, _vMaxProjPos = _vCenterProjPos;

//        //Each voxel corner
//        for(uint j = 0; j < 8; ++j)
//        {
//            vec3 _vCorner = vec3(_voxel.corners[j*3], _voxel.corners[j*3+1], _voxel.corners[j*3+2]);

//            //Project corner
//            float _vCornerProjDistancebb = dot(_vCorner -_colliderbb, _currentAxis);
//            vec3 _vCornerProjPos = _colliderbb + _vCornerProjDistancebb * _currentAxis;

//            if(_vCornerProjDistancebb < _vMinCoordOnProjAxis)
//            {
//                _vMinCoordOnProjAxis = _vCornerProjDistancebb;
//                _vMinProjPos = _vCornerProjPos;
//            }

//            else if(_vCornerProjDistancebb > _vMaxCoordOnProjAxis)
//            {
//                _vMaxCoordOnProjAxis = _vCornerProjDistancebb;
//                _vMaxProjPos = _vCornerProjPos;
//            }
//        }

//        bool _vMinAftercMax = _vMinCoordOnProjAxis > _cBBProjDistancebb;
//        bool _vMaxBeforecMin =  _vMaxCoordOnProjAxis < _cbbProjDistancebb;
//        _isColliding = _isColliding && !(_vMinAftercMax || _vMaxBeforecMin);
//    }
//    return _isColliding;
}

void FillVoxelWithCubeColliders()
{
    //Get voxel of this group
    uint _index = GetVoxelThreadIndex();
    atomicExchange(voxels[_index].mNbCubeCollider, 0);

    //Get nb threads available on X (for a collider)
    uint _threadsInGroupX= gl_WorkGroupSize.x;

    //Synchronize all threads usefull to prevent a thread making mNbCubeCollider = 0 if particle has been added by another thread
    barrier();

    //Less cubeColliders than threads or equals
    if(nbCubeColliders<=_threadsInGroupX)
    {
        //Get thread index on X and return if useless
        uint _cubeColliderIndex = gl_LocalInvocationID.x;
        if(_cubeColliderIndex >= nbCubeColliders) return;

        //Check if cube collider in voxel with group of index _cubeColliderIndex on X
        if(!IsCubeColliderInVoxel(voxels[_index], cubeColliders[_cubeColliderIndex])) return;

        //barrier();

        //Add each thread particle index and increment mNbCubeCollider
        atomicExchange(indicesCubeColliderVoxelsBuffer[_index*MAX_CUBE_COLLIDERS+voxels[_index].mNbCubeCollider], _cubeColliderIndex);
        atomicAdd(voxels[_index].mNbCubeCollider,1);
    }

    //More particle than thread (multiple of threads)
//    else
//    {
//        //Get nb cubeColliders by thread available
//        uint _cubeColliderPerThread = nbCubeColliders/_threadsInGroup;
//        //Get thread offset in all cubeColliders' table
//        uint _threadOffset = _cubeColliderPerThread*gl_LocalInvocationIndex;

//        //For each cube collider per thread
//        for(uint i = 0; i < _cubeColliderPerThread; ++i)
//        {
//            //Return if useless or particle not in voxel
//            if(i+_threadOffset >= nbCubeColliders) return;
//            if(!IsCubeColliderInVoxel(voxels[_index], cubeColliders[i+_threadOffset])) continue;

//            //Add each thread particle index and increment mNbCubeCollider
//            atomicExchange(indicesCubeColliderVoxelsBuffer[_index*MAX_CUBE_COLLIDERS+voxels[_index].mNbCubeCollider], i+_threadOffset);
//            atomicAdd(voxels[_index].mNbCubeCollider,1);
//        }
//    }


//    for(uint i = 0; i < nbCubeColliders; ++i)
//    {
//        if(!IsCubeColliderInVoxel(voxels[_index], cubeColliders[i])) continue;
//        voxels[_index].mCubeCollider[voxels[_index].mNbCubeCollider++] = i;
//        if(voxels[_index].mNbCubeCollider >= MAX_CUBE_COLLIDERS) return;
//    }

    //barrier();
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    CreateGrid();
    FillVoxelWithParticles();
    FillVoxelWithCubeColliders();
}
