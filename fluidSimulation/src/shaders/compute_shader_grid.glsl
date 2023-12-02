#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Voxel
{
    float mbbX;
    float mbbY;
    float mbbZ;

    float mBBX;
    float mBBY;
    float mBBZ;

    uint mAllParticles[1000];
    uint mNbParticles;
};

layout(std430, binding = 0) buffer VoxelBuffer
{
    Voxel voxels[];
};

struct Particle
{
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

uniform float stepX;
uniform float stepY;
uniform float stepZ;

uniform float bbX;
uniform float bbY;
uniform float bbZ;

uniform uint sizeGrid;
uniform uint nbParticles;

bool IsParticleInVoxel(const Voxel _voxel, const Particle _particle)
{
    bool _inXaxis = _voxel.mbbX <= _particle.positionX && _particle.positionX <= _voxel.mBBX;
    bool _inYaxis = _voxel.mbbY <= _particle.positionY && _particle.positionY <= _voxel.mBBY;
    bool _inZaxis = _voxel.mbbZ <= _particle.positionZ && _particle.positionZ <= _voxel.mBBZ;
    return _inXaxis && _inYaxis && _inZaxis;
}


uint GetVoxelIndex()
{
    uint indexI = gl_GlobalInvocationID.x;
    uint indexJ = gl_GlobalInvocationID.y;
    uint indexK = gl_GlobalInvocationID.z;
    return indexK*sizeGrid*sizeGrid + indexJ*sizeGrid + indexI;
}

void CreateGrid(uint _index)
{
    uint indexI = gl_GlobalInvocationID.x;
    uint indexJ = gl_GlobalInvocationID.y;
    uint indexK = gl_GlobalInvocationID.z;

    voxels[_index].mbbX = bbX + indexI*stepX;
    voxels[_index].mBBX = bbX + (indexI+1)*stepX;

    voxels[_index].mbbY = bbY + indexJ*stepY;
    voxels[_index].mBBY = bbY + (indexJ+1)*stepY;

    voxels[_index].mbbZ = bbZ + indexK*stepZ;
    voxels[_index].mBBZ = bbZ + (indexK+1)*stepZ;
}

void FillVoxels(uint _index)
{
    voxels[_index].mNbParticles = 0;
    for(uint i = 0; i < nbParticles; ++i)
    {
        if(!IsParticleInVoxel(voxels[_index], particles[i])) continue;
        voxels[_index].mAllParticles[voxels[_index].mNbParticles++] = i;
    }

}

void main()
{
    uint _index = GetVoxelIndex();
    CreateGrid(_index);
    FillVoxels(_index);
}
