#include "Grid.h"

Grid::Grid()
{

}

Grid::Grid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    mAllVoxels.resize(_nbVoxelPerSide*_nbVoxelPerSide*_nbVoxelPerSide);
    float _stepX = (_BB.x() - _bb.x())/(float)_nbVoxelPerSide;
    float _stepY = (_BB.y() - _bb.y())/(float)_nbVoxelPerSide;
    float _stepZ = (_BB.z() - _bb.z())/(float)_nbVoxelPerSide;
}
