#ifndef GRID_H
#define GRID_H

#include <QVector>
#include <QVector3D>

//      Y
//      |
//      |
//      |_________ X
//     /
//    /
//   /
//  Z

struct Voxel
{
    QVector<unsigned long> mParticlesIndex = QVector<unsigned long>();
    QVector<unsigned long> mObstaclesIndex = QVector<unsigned long>();
};

class Grid
{
    QVector<Voxel> mAllVoxels = QVector<Voxel>();

public:
    Grid();
    Grid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide);
};

#endif // GRID_H
