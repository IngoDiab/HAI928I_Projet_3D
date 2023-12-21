#ifndef MCGRID_H
#define MCGRID_H

#include <QVector3D>
#include <QVector>

//      Y
//      |
//      |
//      |_________ X
//     /
//    /
//   /
//  Z
//      right-handed

//            6             7
//            +-------------+               +-----6-------+
//          / |           / |             / |            /|
//        /   |         /   |          11   7         10   5
//    2 +-----+-------+  3  |         +-----+2------+     |
//      |   4 +-------+-----+ 5       |     +-----4-+-----+
//      |   /         |   /           3   8         1   9
//      | /           | /             | /           | /
//    0 +-------------+ 1             +------0------+

struct MCVoxel
{
    float mbbX = 0, mbbY = 0, mbbZ = 0;
    float mBBX = 0, mBBY = 0, mBBZ = 0;
    float mCorners[24] = { 0 };
    float mCornersDensity[8] = { 0 };
    float mTrianglesPos[45] = { 0 };
    float mTrianglesNormales[15] = { 0 };
    uint mNbTriangles = 0;

    MCVoxel(){}

    MCVoxel(const QVector3D& _bb, const QVector3D& _BB)
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

class MCGrid
{
    bool mDrawGrid = false;
    uint mNbVoxelsPerSide = 1, mNbVoxels = 0;
    float mOffset = 0;//.09f;
    float mStep[3] = {0,0,0};
    QVector3D mbb = QVector3D(), mBB = QVector3D();
    QVector<MCVoxel> mAllVoxels = QVector<MCVoxel>();
    float mSeuil = 2;

public:
    QVector<MCVoxel> const & GetAllVoxels() const {return mAllVoxels;}
    QVector<MCVoxel>& GetAllVoxels() {return mAllVoxels;}

    void SetAllVoxels(const QVector<MCVoxel>& _voxelsProcessed) {mAllVoxels = _voxelsProcessed;}

    uint GetNbVoxels() const {return mAllVoxels.size();}
    uint GetSize() const {return mNbVoxelsPerSide;}
    float GetSeuil() const {return mSeuil;}
    float GetStep(ushort _index) const {return mStep[_index];}
    float Getbb(ushort _index) const {return mbb[_index];}

public:
    MCGrid(uint _nbVoxelPerSide);

public:
    void GenerateBoundingBoxGrid(const QVector3D& _bb, const QVector3D& _BB);
    void GenerateCorners();
    void DrawGrid() const;
    void DisplayVoxel(const MCVoxel& _voxel) const;
};

#endif // MCGRID_H
