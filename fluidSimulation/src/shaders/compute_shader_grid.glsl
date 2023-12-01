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

uniform uint sizeGrid;
uniform uint nbParticles;

bool IsParticleInVoxel(const Voxel _voxel, const Particle _particle)
{
    bool _inXaxis = _voxel.mbbX <= _particle.positionX && _particle.positionX <= _voxel.mBBX;
    bool _inYaxis = _voxel.mbbY <= _particle.positionY && _particle.positionY <= _voxel.mBBY;
    bool _inZaxis = _voxel.mbbZ <= _particle.positionZ && _particle.positionZ <= _voxel.mBBZ;
    return _inXaxis && _inYaxis && _inZaxis;
}

void FillVoxels()
{
    uint indexX = gl_GlobalInvocationID.x;
    uint indexY = gl_GlobalInvocationID.y;
    uint indexZ = gl_GlobalInvocationID.z;
    uint indexArray = indexZ*sizeGrid*sizeGrid + indexY*sizeGrid + indexX;

    voxels[indexArray].mNbParticles = 0;
    for(uint i = 0; i < nbParticles; ++i)
    {
        if(!IsParticleInVoxel(voxels[indexArray], particles[i])) continue;
        voxels[indexArray].mAllParticles[voxels[indexArray].mNbParticles++] = i;
    }

}

void main()
{
    FillVoxels();
}
