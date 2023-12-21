#version 430

////////////////////////////// MARCHING CUBE SHADER //////////////////////////////

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

//////////////////////////////  CONST //////////////////////////////
const int MAX_PARTICLES = 10240;
const float M_PI = 3.14159265358979323846;

const uint NB_THREADS_IN_GROUP = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;

const float SMOOTHING_RADIUS = 1.25f;
const float SMOOTHING_RADIUS_P4 = SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS*SMOOTHING_RADIUS;
const float VOLUME = (M_PI * SMOOTHING_RADIUS_P4) / 6;
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
    Voxel grid[];
};

layout(std430, binding = 1) buffer IndicesParticlesBuffer
{
    uint indicesParticlesVoxelsBuffer[];
};

struct MCVoxel
{
    float mbbX;
    float mbbY;
    float mbbZ;

    float mBBX;
    float mBBY;
    float mBBZ;

    float corners[24];
    float cornersDensity[8];

    float trianglesPos[45];
    float trianglesNormales[15];
    uint nbTriangles;
};

layout(std430, binding = 2) buffer MCBuffer
{
    MCVoxel marchingCube_grid[];
};


layout(std430, binding = 10) buffer MCTableBuffer
{
    int MCTable[];
};

layout(std430, binding = 11) buffer MCEdgeTableBuffer
{
    int MCEdgeTable[];
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
};

layout(std430, binding = 3) buffer ParticleBuffer
{
    Particle particles[];
};

//////////////////////////////  UNIFORMS //////////////////////////////

uniform float stepX;
uniform float stepY;
uniform float stepZ;

uniform float stepMCX;
uniform float stepMCY;
uniform float stepMCZ;

uniform float bbX;
uniform float bbY;
uniform float bbZ;

uniform float bbMCX;
uniform float bbMCY;
uniform float bbMCZ;

uniform float seuil;
uniform uint sizeGrid;
uniform uint sizeMCGrid;
uniform uint nbParticles;

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

    float vbbX = bbMCX + indexI*stepMCX;
    marchingCube_grid[_index].mbbX = vbbX;
    float vBBX = bbMCX + (indexI+1)*stepMCX;
    marchingCube_grid[_index].mBBX = vBBX;

    float vbbY = bbMCY + indexJ*stepMCY;
    marchingCube_grid[_index].mbbY = vbbY;
    float vBBY = bbMCY + (indexJ+1)*stepMCY;
    marchingCube_grid[_index].mBBY = vBBY;

    float vbbZ = bbMCZ + indexK*stepMCZ;
    marchingCube_grid[_index].mbbZ = vbbZ;
    float vBBZ = bbMCZ + (indexK+1)*stepMCZ;
    marchingCube_grid[_index].mBBZ = vBBZ;

    uint _cornerIndex = gl_LocalInvocationIndex;
    if(_cornerIndex>=8)return;
    marchingCube_grid[_index].corners[3*_cornerIndex] = _cornerIndex%2 == 0 ? vbbX : vBBX;
    marchingCube_grid[_index].corners[3*_cornerIndex+1] = (_cornerIndex%4 == 0 || _cornerIndex%4 == 1) ? vbbY : vBBY;
    marchingCube_grid[_index].corners[3*_cornerIndex+2] = (_cornerIndex/4 == 1) ? vbbZ : vBBZ;
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

//////////////////////////////  PROCESS PARTICLES //////////////////////////////

float SmoothKernel(float _distance)
{
    if(_distance >= SMOOTHING_RADIUS) return 0;
    float _difference = SMOOTHING_RADIUS - _distance;
    return _difference * _difference / VOLUME;
}

shared float[NB_THREADS_IN_GROUP] densities = float[NB_THREADS_IN_GROUP](0.f);
//shared uint[NB_THREADS_IN_GROUP] nbParticleContributing = uint[NB_THREADS_IN_GROUP](0);

void CornersDensity()
{
    uint _index = GetVoxelThreadIndex();
    MCVoxel _mcVoxel = marchingCube_grid[_index];

    for(uint _cornerIndex = 0; _cornerIndex < 8; ++_cornerIndex)
    {
        marchingCube_grid[_index].cornersDensity[_cornerIndex] = 0;
        barrier();

        vec3 _cornerPosition = vec3(_mcVoxel.corners[_cornerIndex*3], _mcVoxel.corners[_cornerIndex*3+1], _mcVoxel.corners[_cornerIndex*3+2]);

        //Get corner coord on search grid
        vec3 _gridPosition = XYZCoordToGridCoord(_cornerPosition);

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

        float _densityCorner = 0;
        //uint _nbParticlesContributing = 0;

        //For each close voxel
        for(uint _k = _minK; _k <= _maxK; ++_k)
            for(uint _j = _minJ; _j <= _maxJ; ++_j)
                for(uint _i = _minI; _i <= _maxI; ++_i)
                {
                    uint _voxelIndex = GridCoordToLinearCoord(_i, _j, _k);
                    uint _nbParticlesInVoxel = grid[_voxelIndex].mNbParticles;
                    uint _nbParticlesPerThread = _nbParticlesInVoxel / NB_THREADS_IN_GROUP;
                    uint _nbParticlesLeft = _nbParticlesInVoxel % NB_THREADS_IN_GROUP;

                    for(uint _particleIndex = 0; _particleIndex < _nbParticlesPerThread; ++_particleIndex)
                    {
                        uint _offsetThreadIndex = _particleIndex + gl_LocalInvocationIndex * _nbParticlesPerThread;
                        uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                        Particle _particle = particles[_particleRealIndex];
                        vec3 _particlePosition = vec3(_particle.positionX, _particle.positionY, _particle.positionZ);
                        float _distanceParticleToCorner = length(_cornerPosition - _particlePosition);
//                        if(_distanceParticleToCorner > SMOOTHING_RADIUS) continue;
//                        ++_nbParticlesContributing;
//                        _densityCorner += _particle.density;
                        float _influence = SmoothKernel(_distanceParticleToCorner);
                        _densityCorner += _influence;
                    }

                    uint _nbParticlesProcessed = _nbParticlesPerThread * NB_THREADS_IN_GROUP;

                    if(gl_LocalInvocationIndex < _nbParticlesLeft)
                    {
                        uint _offsetThreadIndex = _nbParticlesProcessed + gl_LocalInvocationIndex;
                        uint _particleRealIndex = indicesParticlesVoxelsBuffer[_voxelIndex*MAX_PARTICLES+_offsetThreadIndex];
                        Particle _particle = particles[_particleRealIndex];
                        vec3 _particlePosition = vec3(_particle.positionX, _particle.positionY, _particle.positionZ);
                        float _distanceParticleToCorner = length(_cornerPosition - _particlePosition);
//                        if(_distanceParticleToCorner > SMOOTHING_RADIUS) continue;
//                        ++_nbParticlesContributing;
//                        _densityCorner += _particle.density;
                        float _influence = SmoothKernel(_distanceParticleToCorner);
                        _densityCorner += _influence;
                    }
                }

        //nbParticleContributing[gl_LocalInvocationIndex] = _nbParticlesContributing;
        densities[gl_LocalInvocationIndex] = _densityCorner;

        barrier();
//        if(gl_LocalInvocationIndex == 0)
//        {
//            uint _totalParticleContributing = 0;
//            for(uint i = 0; i < NB_THREADS_IN_GROUP; ++i)
//                _totalParticleContributing += nbParticleContributing[i];

//            if(_totalParticleContributing != 0)
//            {
//                for(uint i = 0; i < NB_THREADS_IN_GROUP; ++i)
//                    marchingCube_grid[_index].cornersDensity[_cornerIndex] += densities[i]/float(_totalParticleContributing);
//            }

//            densities = float[NB_THREADS_IN_GROUP](0.f);
//            nbParticleContributing = uint[NB_THREADS_IN_GROUP](0);
//        }
//        barrier();

        if(gl_LocalInvocationIndex == 0) {

        for(uint i = 0; i < NB_THREADS_IN_GROUP; ++i)
            marchingCube_grid[_index].cornersDensity[_cornerIndex] += densities[i];
        }
        barrier();
    }
}


void CreateTriangles()
{
    if(gl_LocalInvocationIndex != 0) return;

    uint _index = GetVoxelThreadIndex();
    marchingCube_grid[_index].trianglesPos = float[45](0.f);
    marchingCube_grid[_index].trianglesNormales = float[15](0.f);
    marchingCube_grid[_index].nbTriangles = 0;

    MCVoxel _mcVoxel = marchingCube_grid[_index];
    uint _nbTrianglesInVoxel = 0;

    int _configurationVoxel = 0;
    for(int _cornerIndex = 0; _cornerIndex < 8; ++_cornerIndex)
        if(_mcVoxel.cornersDensity[_cornerIndex] < seuil)
            _configurationVoxel |= 1 << _cornerIndex;

    uint _startIndexInMCTable = _configurationVoxel * 16;
    uint _endIndexInMCTable = (_configurationVoxel+1) * 16;
    for(uint _edgeIndexInMCTable = _startIndexInMCTable; _edgeIndexInMCTable < _endIndexInMCTable; _edgeIndexInMCTable+=3)
    {
        uint _edge0Index = MCTable[_edgeIndexInMCTable];
        uint _edge1Index = MCTable[_edgeIndexInMCTable +2];
        uint _edge2Index = MCTable[_edgeIndexInMCTable +1];
        if(_edge0Index == -1 || _edge1Index == -1 || _edge2Index == -1) break;

        vec3 _vertex0 = vec3(0,0,0);
        vec3 _vertex1 = vec3(0,0,0);
        vec3 _vertex2 = vec3(0,0,0);

        //Construct triangle with 3 edges
        for(uint _vertexIndex = 0; _vertexIndex < 3; ++_vertexIndex)
        {
            uint _edgeIndex = _vertexIndex == 0 ? _edge0Index : (_vertexIndex == 1 ? _edge1Index : _edge2Index);
            uint _edgeInMCEdgeTable = _edgeIndex * 2;
            uint _cornerAIndex = MCEdgeTable[_edgeInMCEdgeTable];
            vec3 _cornerAPosition = vec3(_mcVoxel.corners[_cornerAIndex*3], _mcVoxel.corners[_cornerAIndex*3+1], _mcVoxel.corners[_cornerAIndex*3+2]);

            uint _cornerBIndex = MCEdgeTable[_edgeInMCEdgeTable+1];
            vec3 _cornerBPosition = vec3(_mcVoxel.corners[_cornerBIndex*3], _mcVoxel.corners[_cornerBIndex*3+1], _mcVoxel.corners[_cornerBIndex*3+2]);

            //TODO Opti ?
//            float _densityA = _mcVoxel.cornersDensity[_cornerAIndex];
//            float _densityB = _mcVoxel.cornersDensity[_cornerBIndex];
//            float _minDensity = min(_densityA, _densityB);
//            float _maxDensity = max(_densityA, _densityB);
//            float _percentInterpo = (seuil - _minDensity) / (_maxDensity - _minDensity);
//            vec3 _currentVertex = _cornerAPosition + (_cornerBPosition - _cornerAPosition)*_percentInterpo;
            vec3 _currentVertex = (_cornerAPosition+_cornerBPosition)/2.f;
            //

            uint _indexVertexInTriangles = _nbTrianglesInVoxel*9 + _vertexIndex*3;
            marchingCube_grid[_index].trianglesPos[_indexVertexInTriangles] = _currentVertex.x;
            marchingCube_grid[_index].trianglesPos[_indexVertexInTriangles +1] = _currentVertex.y;
            marchingCube_grid[_index].trianglesPos[_indexVertexInTriangles +2] = _currentVertex.z;

            if(_vertexIndex==0) _vertex0 = _currentVertex;
            else if(_vertexIndex==1) _vertex1 = _currentVertex;
            else if(_vertexIndex==2) _vertex2 = _currentVertex;
        }

        vec3 _normale = cross(normalize(_vertex2-_vertex0), normalize(_vertex1-_vertex0));
        marchingCube_grid[_index].trianglesNormales[_nbTrianglesInVoxel*3] = _normale.x;
        marchingCube_grid[_index].trianglesNormales[_nbTrianglesInVoxel*3 +1] = _normale.y;
        marchingCube_grid[_index].trianglesNormales[_nbTrianglesInVoxel*3 +2] = _normale.z;

        ++_nbTrianglesInVoxel;
    }

    marchingCube_grid[_index].nbTriangles = _nbTrianglesInVoxel;

}
//////////////////////////////  MAIN //////////////////////////////

void main()
{
    CreateGrid();
    CornersDensity();
    CreateTriangles();
}
