#include "Grid.h"
#include "QtDebug"
#include "../Particle/Particle.h"
#include <cmath>
using namespace std;

Grid::Grid(){}

Grid::Grid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    GenerateGrid(_bb, _BB, _nbVoxelPerSide);
}

void Grid::GenerateGrid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    mNbVoxelsPerSide = _nbVoxelPerSide;
    mNbVoxels = _nbVoxelPerSide*_nbVoxelPerSide*_nbVoxelPerSide;
    mAllVoxels.resize(mNbVoxels);

    mbb = _bb - QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);
    mBB = _BB + QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);

    mStep[0] = (mBB.x() - mbb.x())/(float)_nbVoxelPerSide;
    mStep[1] = (mBB.y() - mbb.y())/(float)_nbVoxelPerSide;
    mStep[2] = (mBB.z() - mbb.z())/(float)_nbVoxelPerSide;

//    for(unsigned int _k = 0; _k < mNbVoxelsPerSide; ++_k)
//        for(unsigned int _j = 0; _j < mNbVoxelsPerSide; ++_j)
//            for(unsigned int _i = 0; _i < mNbVoxelsPerSide; ++_i)
//            {
//                uint _arrayIndex = GridCoordToLinearCoord(_i, _j, _k);
//                Voxel& _voxel = mAllVoxels[_arrayIndex];
//                _voxel.mbbX = mbb.x() + _i*mStepX;
//                _voxel.mBBX = mbb.x() + (_i+1)*mStepX;

//                _voxel.mbbY = mbb.y() + _j*mStepY;
//                _voxel.mBBY = mbb.y() + (_j+1)*mStepY;

//                _voxel.mbbZ = mbb.z() + _k*mStepZ;
//                _voxel.mBBZ = mbb.z() + (_k+1)*mStepZ;
//            }
}

void Grid::DrawGrid()
{
    for(const Voxel& _voxel : mAllVoxels)
    {
        vector<QVector3D> corners = vector<QVector3D>(8);
        const QVector3D _topNearLeft = QVector3D(_voxel.mbbX, _voxel.mBBY, _voxel.mBBZ);
        corners[0] = _topNearLeft;

        const QVector3D _bottomNearLeft = QVector3D(_voxel.mbbX, _voxel.mbbY, _voxel.mBBZ);
        corners[1] = _bottomNearLeft;

        const QVector3D _topNearRight = QVector3D(_voxel.mBBX,_voxel.mBBY, _voxel.mBBZ);
        corners[2] = _topNearRight;

        const QVector3D _bottomNearRight = QVector3D(_voxel.mBBX,_voxel.mbbY, _voxel.mBBZ);
        corners[3] = _bottomNearRight;

        const QVector3D _topFarLeft = QVector3D(_voxel.mbbX, _voxel.mBBY, _voxel.mbbZ);
        corners[4] = _topFarLeft;

        const QVector3D _bottomFarLeft = QVector3D(_voxel.mbbX, _voxel.mbbY, _voxel.mbbZ);
        corners[5] = _bottomFarLeft;

        const QVector3D _topFarRight = QVector3D(_voxel.mBBX,_voxel.mBBY, _voxel.mbbZ);
        corners[6] = _topFarRight;

        const QVector3D _bottomFarRight = QVector3D(_voxel.mBBX,_voxel.mbbY, _voxel.mbbZ);
        corners[7] = _bottomFarRight;

        if(_voxel.mNbParticles <1)continue;
        _voxel.mNbParticles > 0 ? glColor3f(1,0,0) : glColor3f(0,0,0);
        DisplayVoxel(corners);
    }
}

void Grid::DisplayVoxel(const vector<QVector3D>& corners) const
{
    DrawFace(0,1,3,2, corners); //Front
    DrawFace(4,5,7,6, corners); //Back
    DrawFace(0,1,5,4, corners); //Left
    DrawFace(2,3,7,6, corners); //Right
    DrawFace(0,2,6,4, corners); //Top
    DrawFace(1,3,7,5, corners); //Bottom
}

void Grid::DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3, const unsigned int& _index4, const vector<QVector3D>& corners) const
{
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_LOOP);
    glVertex3f(corners[_index1].x(), corners[_index1].y(), corners[_index1].z());
    glVertex3f(corners[_index2].x(), corners[_index2].y(), corners[_index2].z());
    glVertex3f(corners[_index3].x(), corners[_index3].y(), corners[_index3].z());
    glVertex3f(corners[_index4].x(), corners[_index4].y(), corners[_index4].z());
    glEnd();
    glEnable(GL_LIGHTING);
}

uint Grid::GridCoordToLinearCoord(uint _i, uint _j, uint _k) const
{
    return _k*mNbVoxelsPerSide*mNbVoxelsPerSide + _j*mNbVoxelsPerSide + _i;
}

uint Grid::GridCoordToLinearCoord(const QVector3D& _position) const
{
    return _position.z()*mNbVoxelsPerSide*mNbVoxelsPerSide + _position.y()*mNbVoxelsPerSide + _position.x();
}

QVector3D Grid::XYZCoordToGridCoord(const QVector3D& _position) const
{
    float _distanceX = mBB.x() - mbb.x();
    float _distanceY = mBB.y() - mbb.y();
    float _distanceZ = mBB.z() - mbb.z();
    const unsigned int _i = ((_position.x()-mbb.x())/_distanceX)*mNbVoxelsPerSide;
    const unsigned int _j = ((_position.y()-mbb.y())/_distanceY)*mNbVoxelsPerSide;
    const unsigned int _k = ((_position.z()-mbb.z())/_distanceZ)*mNbVoxelsPerSide;
    return QVector3D(_i, _j, _k);
}

//void Grid::PutInVoxels(const ParticleComputableData& _particle, unsigned int _index)
//{
//    for(Voxel& _voxel : mAllVoxels)
//    {
//        if(!IsParticleInVoxel(_voxel, _particle)) continue;
//        _voxel.mAllParticles[_voxel.mNbParticles++] = _index;
//    }
//}

//bool Grid::IsParticleInVoxel(const Voxel& _voxel, const ParticleComputableData& _particle)
//{
//    bool _inXaxis = _voxel.mbbX <= _particle.mPositionX && _particle.mPositionX <= _voxel.mBBX;
//    bool _inYaxis = _voxel.mbbY <= _particle.mPositionY && _particle.mPositionY <= _voxel.mBBY;
//    bool _inZaxis = _voxel.mbbZ <= _particle.mPositionZ && _particle.mPositionZ <= _voxel.mBBZ;
//    return _inXaxis && _inYaxis && _inZaxis;
//}

QVector<uint> Grid::GetVoxelIndicesInRange(const QVector3D& _position, float _distance) const
{
    int _nbVoxelX = ceil(_distance/mStep[0]);
    int _nbVoxelY = ceil(_distance/mStep[1]);
    int _nbVoxelZ = ceil(_distance/mStep[2]);
    QVector3D _gridPosition = XYZCoordToGridCoord(_position);
    QVector<uint> _voxels = QVector<uint>();

    uint _minK = max(_gridPosition.z() - _nbVoxelZ, 0.f);
    uint _minJ = max(_gridPosition.y() - _nbVoxelY, 0.f);
    uint _minI = max(_gridPosition.x() - _nbVoxelX, 0.f);

    uint _maxK = min(_gridPosition.z() + _nbVoxelZ, mNbVoxelsPerSide - 1.f);
    uint _maxJ = min(_gridPosition.y() + _nbVoxelY, mNbVoxelsPerSide - 1.f);
    uint _maxI = min(_gridPosition.x() + _nbVoxelX, mNbVoxelsPerSide - 1.f);

    for(unsigned int _k = _minK; _k <= _maxK; ++_k)
        for(unsigned int _j = _minJ; _j <= _maxJ; ++_j)
            for(unsigned int _i = _minI; _i <= _maxI; ++_i)
            {
                uint _linearCoord = GridCoordToLinearCoord(_i, _j, _k);
                //if(_linearCoord < 0 || _linearCoord >= mNbVoxels) continue;
                _voxels.push_back(_linearCoord);
            }
    return _voxels;
}
