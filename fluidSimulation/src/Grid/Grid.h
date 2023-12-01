#ifndef GRID_H
#define GRID_H

#include <vector>
using namespace std;
#include <QGLWidget>
#include <QVector3D>

//      Y
//      |
//      |
//      |_________ X
//     /
//    /
//   /
//  Z
//      right-handed

class ParticleComputableData;

struct Voxel
{
    float mbbX, mbbY, mbbZ = 0;

    float mBBX, mBBY, mBBZ = 0;

    uint mAllParticles[1000];
    uint mNbParticles = 0;

    Voxel(){}

    Voxel(const QVector3D& _bb, const QVector3D& _BB)
    {
        mbbX = _bb.x();
        mbbY = _bb.y();
        mbbZ = _bb.z();

        mBBX = _BB.x();
        mBBY = _BB.y();
        mBBZ = _BB.z();
    }
};

class Grid
{
    unsigned int mNbVoxelPerSide = 0;
    float mOffset = 0.001;
    QVector<Voxel> mAllVoxels = QVector<Voxel>();

public:
    QVector<Voxel> const & GetAllVoxels() const {return mAllVoxels;}
    Voxel& GetVoxel(uint _index) {return mAllVoxels[_index];}
    void SetAllVoxels(const QVector<Voxel>& _voxelsProcessed) {mAllVoxels = _voxelsProcessed;}

    uint GetNbVoxels() const {return mAllVoxels.size();}

public:
    Grid();
    Grid(QVector3D& _bb, QVector3D& _BB, unsigned int _nbVoxelPerSide);

public:
    void GenerateGrid(QVector3D& _bb, QVector3D& _BB, unsigned int _nbVoxelPerSide);
    void DrawGrid();
    void DisplayVoxel(const vector<QVector3D>& corners) const;
    void DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3, const unsigned int& _index4, const vector<QVector3D>& corners) const;
    uint XYZCoordToLinearCoord(uint _x, uint _y, uint _z);

    //CS
    void PutInVoxels(const ParticleComputableData& _particle, unsigned int _index);
    bool IsParticleInVoxel(const Voxel& _voxel, const ParticleComputableData& _particle);
};

#endif // GRID_H
