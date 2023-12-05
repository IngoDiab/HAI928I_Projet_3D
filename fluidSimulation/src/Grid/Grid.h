#ifndef GRID_H
#define GRID_H

#include <vector>
using namespace std;
#include <QGLWidget>
#include <QVector3D>

#include "../Utils/Macros.h"

//      Y
//      |
//      |
//      |_________ X
//     /
//    /
//   /
//  Z
//      right-handed

class CubeCollider;

struct Voxel
{
    float mbbX, mbbY, mbbZ = 0;

    float mBBX, mBBY, mBBZ = 0;

    uint mAllParticles[NB_PARTICLES];
    uint mNbParticles = 0;

    uint mCubeCollider[10];
    uint mNbCubeCollider = 0;

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

/////////////////////////////////////////////////////////

class Grid
{
    uint mNbVoxelsPerSide = 0, mNbVoxels = 0;
    float mOffset = .1;
    float mStep[3] = {0,0,0};
    QVector3D mbb = QVector3D(), mBB = QVector3D();
    QVector<Voxel> mAllVoxels = QVector<Voxel>();
    QVector<QVector<QVector3D>> mCorners = QVector<QVector<QVector3D>>();

public:
    QVector<Voxel> const & GetAllVoxels() const {return mAllVoxels;}
    QVector<QVector3D> GetCorners(int _index) const {return mCorners[_index];}
    Voxel& GetVoxel(uint _index) {return mAllVoxels[_index];}
    void SetAllVoxels(const QVector<Voxel>& _voxelsProcessed) {mAllVoxels = _voxelsProcessed;}

    uint GetNbVoxels() const {return mAllVoxels.size();}
    float GetStep(ushort _index) const {return mStep[_index];}
    float Getbb(ushort _index) const {return mbb[_index];}

public:
    Grid();
    Grid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide);

public:
    void GenerateBoundingBoxGrid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide);
    void DrawGrid();
    void DisplayVoxel(const QVector<QVector3D>& corners) const;
    void DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3, const unsigned int& _index4, const QVector<QVector3D>& corners) const;
    uint GridCoordToLinearCoord(uint _i, uint _j, uint _k) const;
    uint GridCoordToLinearCoord(const QVector3D& _position) const;
    QVector3D XYZCoordToGridCoord(const QVector3D& _position) const;

    //CS
    QVector<uint> GetVoxelIndicesInRange(const QVector3D& _position, float _distance) const;
    void PutInVoxels(const CubeCollider& _cube, unsigned int _indexCollider);
    bool IsColliderInVoxel(const Voxel& _voxel, int _indexVoxel, const CubeCollider& _collider);
};

#endif // GRID_H
