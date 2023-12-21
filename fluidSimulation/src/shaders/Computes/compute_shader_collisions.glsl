#version 430

////////////////////////////// COLLISIONS SHADER //////////////////////////////

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_CUBE_COLLIDERS = 10;
const float FLOAT_MAX = 3.4028235e+38;

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

//////////////////////////////  COLLIDERS //////////////////////////////

struct CubeCollider
{
    int placeHolderTransform;
    float mVelocityX;
    float mVelocityY;
    float mVelocityZ;
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

float GetXAxisCollisionLength(CubeCollider _collider) {return length(GetCorner(1, _collider) - GetCorner(0, _collider));}
float GetYAxisCollisionLength(CubeCollider _collider) {return length(GetCorner(5, _collider) - GetCorner(0, _collider));}
float GetZAxisCollisionLength(CubeCollider _collider) {return length(GetCorner(2, _collider) - GetCorner(0, _collider));}

//////////////////////////////  UNIFORMS //////////////////////////////
uniform float deltaTime;

uniform float stepX;
uniform float stepY;
uniform float stepZ;

uniform float bbX;
uniform float bbY;
uniform float bbZ;

uniform uint sizeGrid;
uniform uint nbCubeColliders;
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

//////////////////////////////  COLLISION //////////////////////////////

struct CollisionData
{
    bool isColliding;
    CubeCollider collider;
    vec3 normale;
    float distance;
};

bool Contains(uint _alreadyProcessedColliders[MAX_CUBE_COLLIDERS], uint _nbAlreadyProcessedColliders, uint _valueToCheck)
{
    for(uint i = 0; i < _nbAlreadyProcessedColliders; ++i)
        if(_alreadyProcessedColliders[i] == _valueToCheck) return true;
    return false;
}

float Resolution(CubeCollider _collider, Particle _currentParticle)
{
    vec3 _previousPos = vec3(_currentParticle.previousPositionX, _currentParticle.previousPositionY, _currentParticle.previousPositionZ);
    vec3 _position = vec3(_currentParticle.positionX, _currentParticle.positionY, _currentParticle.positionZ);
    vec3 _movementDir = normalize(_position - _previousPos);

    float _minT = FLOAT_MAX;

    for(uint i = 0; i < 6; ++i)
    {
        vec3 _corner = i/3 == 0 ? Getbb(_collider) : GetBB(_collider);
        vec3 _normalPlane = (i<3 ? 1 : -1) * i%3 == 0 ? GetXAxisCollision(_collider) : (i%3 == 1 ? GetYAxisCollision(_collider) : GetZAxisCollision(_collider));

        //Check plan not parallel to velocity
        float _dotVeloNormal = dot(_movementDir, _normalPlane);
        if((_dotVeloNormal <= 0.01f  && _dotVeloNormal >= -0.01f) || isnan(_dotVeloNormal) || isinf(_dotVeloNormal)) continue;

        float _t = dot((_corner - _previousPos), _normalPlane) / dot(_movementDir, _normalPlane);
        if(_t < -0.01f) continue;

        _minT = min(_minT, _t);
    }

    return _minT;
}


CollisionData Detection(CubeCollider _collider, Particle _currentParticle)
{
    CollisionData _data;
    _data.isColliding = false;

    vec3 _position = vec3(_currentParticle.positionX, _currentParticle.positionY, _currentParticle.positionZ);
    vec3 _previousPos = vec3(_currentParticle.previousPositionX, _currentParticle.previousPositionY, _currentParticle.previousPositionZ);

    //Get bb
    vec3 _colliderbb = Getbb(_collider);

    //Get dimensions cube
    float _sizeX = GetXAxisCollisionLength(_collider);
    float _sizeY = GetYAxisCollisionLength(_collider);
    float _sizeZ = GetZAxisCollisionLength(_collider);

    //Get relatives axis cube
    vec3 _right = GetXAxisCollision(_collider);
    vec3 _up = GetYAxisCollision(_collider);
    vec3 _forward = GetZAxisCollision(_collider);

    //Project particle on all axis
    float _projOnRight = dot(_position - _colliderbb, _right);
    float _projOnUp = dot(_position - _colliderbb, _up);
    float _projOnForward = dot(_position - _colliderbb, _forward);

    bool _collidingOnX = _projOnRight >= 0 && _projOnRight <= _sizeX;
    bool _collidingOnY = _projOnUp >= 0 && _projOnUp <= _sizeY;
    bool _collidingOnZ = _projOnForward >= 0 && _projOnForward <= _sizeZ;
    bool _isInCollider = _collidingOnX && _collidingOnY && _collidingOnZ;
    //return _projOnRight >= 0 && _projOnRight <= _sizeX && _projOnUp >= 0 && _projOnUp <= _sizeY && _projOnForward >= 0 && _projOnForward <= _sizeZ;

    if(_isInCollider)
    {
        //LEFT RIGHT BOT TOP BACK FRONT
        float[6] _distancePene = float[6](_projOnRight, _sizeX-_projOnRight, _projOnUp, _sizeY-_projOnUp, _projOnForward, _sizeZ-_projOnForward);
        float _minDistance = _distancePene[0];
        uint _index = 0;
        for(uint i = 1; i < 6; ++i)
        {
            if(_minDistance <= _distancePene[i]) continue;
            _minDistance = _distancePene[i];
            _index = i;
        }
        vec3 _normale = _index/2 == 0 ? _right : _index/2 == 1 ? _up : _forward;

        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = _index%2 == 0 ? -_normale : _normale;
        _data.distance = _minDistance;
        return _data;
    }

    //Project particle on all axis
    float _previousProjOnRight = dot(_previousPos - _colliderbb, _right);
    float _previousProjOnUp = dot(_previousPos - _colliderbb, _up);
    float _previousProjOnForward = dot(_previousPos - _colliderbb, _forward);

    bool _collideFromLeft = _previousProjOnRight < 0 && _projOnRight >= 0 && _collidingOnY && _collidingOnZ;
    if(_collideFromLeft)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = -_right;
        _data.distance = _projOnRight;
        return _data;
    }
    bool _collideFromRight = _previousProjOnRight > _sizeX && _projOnRight <= _sizeX && _collidingOnY && _collidingOnZ;
    if(_collideFromRight)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = _right;
        _data.distance = _sizeX-_projOnRight;
        return _data;
    }

    bool _collideFromBottom = _previousProjOnUp < 0 && _projOnUp >= 0 && _collidingOnX && _collidingOnZ;
    if(_collideFromBottom)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = -_up;
        _data.distance = _projOnUp;
        return _data;
    }
    bool _collideFromTop = _previousProjOnUp > _sizeY && _projOnUp <= _sizeY && _collidingOnX && _collidingOnZ;
    if(_collideFromTop)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = _up;
        _data.distance = _sizeY-_projOnUp;
        return _data;
    }

    bool _collideFromBack = _previousProjOnForward < 0 && _projOnForward >= 0 && _collidingOnX && _collidingOnY;
    if(_collideFromBack)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = -_forward;
        _data.distance = _projOnForward;
        return _data;
    }
    bool _collideFromFront = _previousProjOnForward > _sizeZ && _projOnForward <= _sizeZ && _collidingOnX && _collidingOnY;
    if(_collideFromFront)
    {
        _data.isColliding = true;
        _data.collider = _collider;
        _data.normale = _forward;
        _data.distance = _sizeZ-_projOnForward;
        return _data;
    }

    return _data;
//    return _collideFromLeft || _collideFromRight || _collideFromBottom || _collideFromTop || _collideFromBack || _collideFromFront;
}

void Collisions(float _distance, Particle _currentParticle)
{
    uint index = GetParticleIndex();

    ///////////////////////////////////////////////////////////////////////////////////////
    bool _needCollision = false;
    for(int i = 0; i < nbCubeColliders; ++i)
    {
        CollisionData _collisionData = Detection(cubeColliders[i], _currentParticle);
        if(!_collisionData.isColliding) continue;
        _needCollision = true;
        particles[index].positionX += _collisionData.normale.x * _collisionData.distance;
        particles[index].positionY += _collisionData.normale.y * _collisionData.distance;
        particles[index].positionZ += _collisionData.normale.z * _collisionData.distance;

        vec3 _velocity = vec3(_currentParticle.velocityX, _currentParticle.velocityY, _currentParticle.velocityZ);
        vec3 _reflected = reflect(_velocity, _collisionData.normale);
        particles[index].velocityX = _reflected.x*0.75f;
        particles[index].velocityY = _reflected.y*0.75f;
        particles[index].velocityZ = _reflected.z*0.75f;

        particles[index].velocityX += cubeColliders[i].mVelocityX;
        particles[index].velocityY += cubeColliders[i].mVelocityY;
        particles[index].velocityZ += cubeColliders[i].mVelocityZ;
    }

//    if(!_needCollision) return;
//    particles[index].velocityX = -particles[index].velocityX*0.98f;
//    particles[index].velocityY = -particles[index].velocityY*0.98f;
//    particles[index].velocityZ = -particles[index].velocityZ*0.98f;


    ///////////////////////////////////////////////////////////////////////////////////////

    //Get grid coord
//    vec3 _gridPosition = XYZCoordToGridCoord(_position);

//    //Get voxel in range
//    int _nbVoxelX = int(ceil(_distance/stepX));
//    int _nbVoxelY = int(ceil(_distance/stepY));
//    int _nbVoxelZ = int(ceil(_distance/stepZ));

//    //Get range from current voxel
//    uint _minK = uint(max(_gridPosition.z - _nbVoxelZ, 0.f));
//    uint _minJ = uint(max(_gridPosition.y - _nbVoxelY, 0.f));
//    uint _minI = uint(max(_gridPosition.x - _nbVoxelX, 0.f));

//    uint _maxK = uint(min(_gridPosition.z + _nbVoxelZ, sizeGrid - 1.f));
//    uint _maxJ = uint(min(_gridPosition.y + _nbVoxelY, sizeGrid - 1.f));
//    uint _maxI = uint(min(_gridPosition.x + _nbVoxelX, sizeGrid - 1.f));

//    //Create buffer to stock already processed colliders
//    uint[MAX_CUBE_COLLIDERS] _alreadyProcessedColliders;
//    uint _nbAlreadyProcessedColliders = 0;

//    //Initialize minimal intersection distance
//    float _minT = FLOAT_MAX;
//    bool _needCollision = false;

//    //For each voxel in range
//    for(uint _k = _minK; _k <= _maxK; ++_k)
//        for(uint _j = _minJ; _j <= _maxJ; ++_j)
//            for(uint _i = _minI; _i <= _maxI; ++_i)
//            {
//                uint _linearCoord = GridCoordToLinearCoord(_i, _j, _k);

//                //Get colliders of this voxel
//                //uint[MAX_CUBE_COLLIDERS] _collidersIndexInVoxel = voxels[_linearCoord].mCubeCollider;
//                uint _nbCollidersIndexInVoxel = voxels[_linearCoord].mNbCubeCollider;

//                //For each colliders of this voxel
//                for(uint _colliderIndexInVoxel = 0; _colliderIndexInVoxel < _nbCollidersIndexInVoxel; ++_colliderIndexInVoxel)
//                {
//                    //Check if this collider has already been used
//                    uint _colliderIndex = indicesCubeColliderVoxelsBuffer[_linearCoord*MAX_CUBE_COLLIDERS+_colliderIndexInVoxel];
//                    if(Contains(_alreadyProcessedColliders, _nbAlreadyProcessedColliders, _colliderIndex)) continue;

//                    //Add to processed list and process it
//                    _alreadyProcessedColliders[_nbAlreadyProcessedColliders++] = _colliderIndex;
//                    CollisionData _collisionData = Detection(cubeColliders[_colliderIndex]);

//                    //If collision, remember intersection distance if nearer to old particle position
//                    if(!_collisionData.isColliding) continue;
//                    _needCollision = true;
////                    _minT = min(_minT, Resolution(cubeColliders[_colliderIndex]));

//                    particles[index].positionX += _collisionData.normale.x * _collisionData.distance;
//                    particles[index].positionY += _collisionData.normale.y * _collisionData.distance;
//                    particles[index].positionZ += _collisionData.normale.z * _collisionData.distance;
//                }
//            }

//    //If there is at least 1 collision
//    if(!_needCollision) return;

//    vec3 _previousPos = vec3(particles[index].previousPositionX, particles[index].previousPositionY, particles[index].previousPositionZ);
////    vec3 _movementDir = normalize(_position - _previousPos);

// //   vec3 _newPos = _previousPos + _minT * _movementDir;
////    particles[index].positionX = _previousPos.x;
////    particles[index].positionY = _previousPos.y;
////    particles[index].positionZ = _previousPos.z;

//    particles[index].velocityX = -particles[index].velocityX/2;
//    particles[index].velocityY = -particles[index].velocityY/2;
//    particles[index].velocityZ = -particles[index].velocityZ/2;
}

//////////////////////////////  MAIN //////////////////////////////

void main()
{
    uint index = GetParticleIndex();
    Particle _currentParticle = particles[index];

    Collisions(1, _currentParticle);
}
