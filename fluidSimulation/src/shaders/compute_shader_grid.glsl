#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 1000;
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

    uint mAllParticles[MAX_PARTICLES];
    uint mNbParticles;

    uint mCubeCollider[MAX_CUBE_COLLIDERS];
    uint mNbCubeCollider;
};

layout(std430, binding = 0) buffer VoxelBuffer
{
    Voxel voxels[];
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

    float velocityX;
    float velocityY;
    float velocityZ;
};

layout(std430, binding = 1) buffer ParticleBuffer
{
    Particle particles[];
};

//////////////////////////////  COLLIDERS //////////////////////////////

struct CubeCollider
{
    int placeHolderTransform;
    float corners[24];
};

layout(std430, binding = 2) buffer CubeColliderBuffer
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

uint GetVoxelIndex()
{
    uint indexI = gl_GlobalInvocationID.x;
    uint indexJ = gl_GlobalInvocationID.y;
    uint indexK = gl_GlobalInvocationID.z;
    return indexK*sizeGrid*sizeGrid + indexJ*sizeGrid + indexI;
}

//////////////////////////////  CREATE VOXELS //////////////////////////////

void CreateGrid(uint _index)
{
    uint indexI = gl_GlobalInvocationID.x;
    uint indexJ = gl_GlobalInvocationID.y;
    uint indexK = gl_GlobalInvocationID.z;

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

    for(uint i = 0; i < 24; i+=3)
    {
        voxels[_index].corners[i] = ((i%12)%4 == 0 || (i%12)%4 == 3) ? vbbX : vBBX;
        voxels[_index].corners[i+1] = (((i+1)%12)%2 == 0) ? vbbY : vBBY;
        voxels[_index].corners[i+2] = ((i+2)/12 == 1) ? vbbZ : vBBZ;
    }

//    voxels[_index].corners[0] = vec3(vbbX, vBBY, vBBZ);
//    voxels[_index].corners[1] = vec3(vbbX, vbbY, vBBZ);
//    voxels[_index].corners[2] = vec3(vBBX, vBBY, vBBZ);
//    voxels[_index].corners[3] = vec3(vBBX, vbbY, vBBZ);

//    voxels[_index].corners[4] = vec3(vbbX, vBBY, vbbZ);
//    voxels[_index].corners[5] = vec3(vbbX, vbbY, vbbZ);
//    voxels[_index].corners[6] = vec3(vBBX, vBBY, vbbZ);
//    voxels[_index].corners[7] = vec3(vBBX, vbbY, vbbZ);
}

//////////////////////////////  PROCESS PARTICLES //////////////////////////////

bool IsParticleInVoxel(const Voxel _voxel, const Particle _particle)
{
    bool _inXaxis = _voxel.mbbX <= _particle.positionX && _particle.positionX <= _voxel.mBBX;
    bool _inYaxis = _voxel.mbbY <= _particle.positionY && _particle.positionY <= _voxel.mBBY;
    bool _inZaxis = _voxel.mbbZ <= _particle.positionZ && _particle.positionZ <= _voxel.mBBZ;
    return _inXaxis && _inYaxis && _inZaxis;
}

void FillVoxelsWithParticles(uint _index)
{
    voxels[_index].mNbParticles = 0;
    for(uint i = 0; i < nbParticles; ++i)
    {
        if(!IsParticleInVoxel(voxels[_index], particles[i])) continue;
        voxels[_index].mAllParticles[voxels[_index].mNbParticles++] = i;
        if(voxels[_index].mNbParticles >= MAX_PARTICLES) return;
    }
}

//////////////////////////////  PROCESS COLLIDERS //////////////////////////////

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

    bool _isColliding = true;
    for(uint i = 0; i < 3; ++i)
    {
        //Project cubeCollider
        float _cCenterProjDistancebb = dot((_colliderbb + _colliderBB/2.0f) -_colliderbb, _axis[i]);
        vec3 _cCenterProjPos = _colliderbb + _cCenterProjDistancebb * _axis[i];
        float _cbbProjDistancebb = dot(_colliderbb -_colliderbb, _axis[i]);
        vec3 _cbbProjPos = _colliderbb + _cbbProjDistancebb * _axis[i];
        float _cBBProjDistancebb = dot(_colliderBB -_colliderbb, _axis[i]);
        vec3 _cBBProjPos = _colliderbb + _cBBProjDistancebb * _axis[i];

        //Project voxel center
        float _vCenterProjDistancebb = dot((_voxelbb + _voxelBB/2.0f ) -_colliderbb, _axis[i]);
        vec3 _vCenterProjPos = _colliderbb + _vCenterProjDistancebb * _axis[i];

        float _vMinCoordOnProjAxis = _vCenterProjDistancebb, _vMaxCoordOnProjAxis = _vCenterProjDistancebb;
        vec3 _vMinProjPos = _vCenterProjPos, _vMaxProjPos = _vCenterProjPos;

        //Each voxel corner
        for(uint j = 0; j < 8; ++j)
        {
            vec3 _vCorner = vec3(_voxel.corners[j*3], _voxel.corners[j*3+1], _voxel.corners[j*3+2]);

            //Project corner
            float _vCornerProjDistancebb = dot(_vCorner -_colliderbb, _axis[i]);
            vec3 _vCornerProjPos = _colliderbb + _vCornerProjDistancebb * _axis[i];

            if(_vCornerProjDistancebb < _vMinCoordOnProjAxis)
            {
                _vMinCoordOnProjAxis = _vCornerProjDistancebb;
                _vMinProjPos = _vCornerProjPos;
            }

            else if(_vCornerProjDistancebb > _vMaxCoordOnProjAxis)
            {
                _vMaxCoordOnProjAxis = _vCornerProjDistancebb;
                _vMaxProjPos = _vCornerProjPos;
            }
        }

        bool _vMinAftercMax = _vMinCoordOnProjAxis > _cBBProjDistancebb;
        bool _vMaxBeforecMin =  _vMaxCoordOnProjAxis < _cbbProjDistancebb;
        _isColliding = _isColliding && !(_vMinAftercMax || _vMaxBeforecMin);
    }

    return _isColliding;
}

void FillVoxelsWithCubeColliders(uint _index)
{
    voxels[_index].mNbCubeCollider = 0;
    for(uint i = 0; i < nbCubeColliders; ++i)
    {
        if(!IsCubeColliderInVoxel(voxels[_index], cubeColliders[i])) continue;
        voxels[_index].mCubeCollider[voxels[_index].mNbCubeCollider++] = i;
        if(voxels[_index].mNbCubeCollider >= MAX_CUBE_COLLIDERS) return;
    }
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    uint _index = GetVoxelIndex();
    CreateGrid(_index);
    FillVoxelsWithParticles(_index);
    FillVoxelsWithCubeColliders(_index);
}
