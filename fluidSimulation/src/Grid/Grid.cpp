#include "Grid.h"
#include "QtDebug"
#include "../Particle/Particle.h"

Grid::Grid()
{

}

Grid::Grid(QVector3D& _bb, QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    GenerateGrid(_bb, _BB, _nbVoxelPerSide);
}

void Grid::GenerateGrid(QVector3D& _bb, QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    mNbVoxelPerSide = _nbVoxelPerSide;
    mAllVoxels.resize(_nbVoxelPerSide*_nbVoxelPerSide*_nbVoxelPerSide);
//    for(Voxel& _voxel : mAllVoxels)
//        _voxel.mNbParticles = 0;

    _bb -= QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);
    _BB += QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);

    float _stepX = (_BB.x() - _bb.x())/(float)_nbVoxelPerSide;
    float _stepY = (_BB.y() - _bb.y())/(float)_nbVoxelPerSide;
    float _stepZ = (_BB.z() - _bb.z())/(float)_nbVoxelPerSide;

    for(unsigned int _z = 0; _z < mNbVoxelPerSide; ++_z)
        for(unsigned int _y = 0; _y < mNbVoxelPerSide; ++_y)
            for(unsigned int _x = 0; _x < mNbVoxelPerSide; ++_x)
            {
                uint _arrayIndex = XYZCoordToLinearCoord(_x, _y, _z);
                Voxel& _voxel = mAllVoxels[_arrayIndex];
                _voxel.mbbX = _bb.x() + _x*_stepX;
                _voxel.mBBX = _bb.x() + (_x+1)*_stepX;

                _voxel.mbbY = _bb.y() + _y*_stepY;
                _voxel.mBBY = _bb.y() + (_y+1)*_stepY;

                _voxel.mbbZ = _bb.z() + _z*_stepZ;
                _voxel.mBBZ = _bb.z() + (_z+1)*_stepZ;
            }
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

uint Grid::XYZCoordToLinearCoord(uint _x, uint _y, uint _z)
{
    return _z*mNbVoxelPerSide*mNbVoxelPerSide + _y*mNbVoxelPerSide + _x;
}

void Grid::PutInVoxels(const ParticleComputableData& _particle, unsigned int _index)
{
    for(Voxel& _voxel : mAllVoxels)
    {
        if(!IsParticleInVoxel(_voxel, _particle)) continue;
        _voxel.mAllParticles[_voxel.mNbParticles++] = _index;
    }
}

//CS
bool Grid::IsParticleInVoxel(const Voxel& _voxel, const ParticleComputableData& _particle)
{
    bool _inXaxis = _voxel.mbbX <= _particle.mPositionX && _particle.mPositionX <= _voxel.mBBX;
    bool _inYaxis = _voxel.mbbY <= _particle.mPositionY && _particle.mPositionY <= _voxel.mBBY;
    bool _inZaxis = _voxel.mbbZ <= _particle.mPositionZ && _particle.mPositionZ <= _voxel.mBBZ;
    return _inXaxis && _inYaxis && _inZaxis;
}
