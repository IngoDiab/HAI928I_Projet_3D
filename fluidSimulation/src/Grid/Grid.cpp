#include "Grid.h"
#include "QtDebug"
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include <cmath>
using namespace std;

Grid::Grid(){}

Grid::Grid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    GenerateBoundingBoxGrid(_bb, _BB, _nbVoxelPerSide);
}

void Grid::GenerateBoundingBoxGrid(const QVector3D& _bb, const QVector3D& _BB, unsigned int _nbVoxelPerSide)
{
    mNbVoxelsPerSide = _nbVoxelPerSide;
    mNbVoxels = _nbVoxelPerSide*_nbVoxelPerSide*_nbVoxelPerSide;
    mAllVoxels.resize(mNbVoxels);

    mbb = _bb - QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);
    mBB = _BB + QVector3D(_nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset, _nbVoxelPerSide*mOffset);

    mStep[0] = (mBB.x() - mbb.x())/(float)_nbVoxelPerSide;
    mStep[1] = (mBB.y() - mbb.y())/(float)_nbVoxelPerSide;
    mStep[2] = (mBB.z() - mbb.z())/(float)_nbVoxelPerSide;
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

        //if(_voxel.mNbCubeCollider < 1) continue;
        _voxel.mNbCubeCollider > 0 ? glColor3f(1,0,0) : glColor3f(0,0,0);
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

void Grid::PutInVoxels(const CubeCollider& _cube, unsigned int _index)
{
    for(Voxel& _voxel : mAllVoxels)
    {
        _voxel.mNbCubeCollider = 0;
        if(!IsColliderInVoxel(_voxel, _cube)) continue;
        _voxel.mCubeCollider[_voxel.mNbCubeCollider++] = _index;
    }
}

bool Grid::IsColliderInVoxel(const Voxel& _voxel, const CubeCollider& _collider)
{
    QVector3D _colliderbb = _collider.Getbb();

    QVector3D _voxelbb = QVector3D(_voxel.mbbX, _voxel.mbbY, _voxel.mbbZ);
    QVector3D _voxelBB = QVector3D(_voxel.mBBX, _voxel.mBBY, _voxel.mBBZ);

    //Get relatives axis cube
    QVector3D _right = _collider.GetXAxisCollision();
    QVector3D _up = _collider.GetYAxisCollision();
    QVector3D _forward = _collider.GetZAxisCollision();

    //Get other corners
    QVector3D _otherCx = _collider.GetOtherXCorner();
    QVector3D _otherCy = _collider.GetOtherYCorner();
    QVector3D _otherCz = _collider.GetOtherZCorner();

    //Project particle on all axis
    QVector3D _bbProjOnRight = _colliderbb + (QVector3D::dotProduct(_voxelbb - _colliderbb, _right) * _right);
    QVector3D _bbProjOnUp = _colliderbb + (QVector3D::dotProduct(_voxelbb - _colliderbb, _up) * _up);
    QVector3D _bbProjOnForward = _colliderbb + (QVector3D::dotProduct(_voxelbb - _colliderbb, _forward) * _forward );

    QVector3D _BBProjOnRight = _colliderbb + (QVector3D::dotProduct(_voxelBB - _colliderbb, _right) * _right);
    QVector3D _BBProjOnUp = _colliderbb + (QVector3D::dotProduct(_voxelBB - _colliderbb, _up) * _up);
    QVector3D _BBProjOnForward = _colliderbb + (QVector3D::dotProduct(_voxelBB - _colliderbb, _forward) * _forward );

    //Check each bb & BB voxel projected if one's orientation is between a pair of collider corner on each axis
    float _dotVbbXCbb = QVector3D::dotProduct((_bbProjOnRight-_colliderbb).normalized(), _right);
    float _dotVBBXCbb = QVector3D::dotProduct((_BBProjOnRight-_colliderbb).normalized(), _right);
    float _dotVbbXCBB = QVector3D::dotProduct((_bbProjOnRight-_otherCx).normalized(), _right);
    float _dotVBBXCBB = QVector3D::dotProduct((_BBProjOnRight-_otherCx).normalized(), _right);

    float _dotVbbYCbb = QVector3D::dotProduct((_bbProjOnUp-_colliderbb).normalized(), _up);
    float _dotVBBYCbb = QVector3D::dotProduct((_BBProjOnUp-_colliderbb).normalized(), _up);
    float _dotVbbYCBB = QVector3D::dotProduct((_bbProjOnUp-_otherCy).normalized(), _up);
    float _dotVBBYCBB = QVector3D::dotProduct((_BBProjOnUp-_otherCy).normalized(), _up);

    float _dotVbbZCbb = QVector3D::dotProduct((_bbProjOnForward-_colliderbb).normalized(), _forward);
    float _dotVBBZCbb = QVector3D::dotProduct((_BBProjOnForward-_colliderbb).normalized(), _forward);
    float _dotVbbZCBB = QVector3D::dotProduct((_bbProjOnForward-_otherCz).normalized(), _forward);
    float _dotVBBZCBB = QVector3D::dotProduct((_BBProjOnForward-_otherCz).normalized(), _forward);

    bool _inXaxis = (_dotVbbXCbb >= 0 && _dotVbbXCBB <= 0) || (_dotVBBXCbb >= 0 &&  _dotVBBXCBB <= 0);
    bool _inYaxis = (_dotVbbYCbb >= 0 && _dotVbbYCBB <= 0) || (_dotVBBYCbb >= 0 &&  _dotVBBYCBB <= 0);
    bool _inZaxis = (_dotVbbZCbb >= 0 && _dotVbbZCBB <= 0) || (_dotVBBZCbb >= 0 &&  _dotVBBZCBB <= 0);

    //If same orientation and bb to projection <= size on dimension for EVERY axis
    return _inXaxis & _inYaxis & _inZaxis;

//    bool _inXaxis = !(_voxel.mBBX < _colliderbb.x() || _voxel.mbbX > _colliderBB.x());
//    bool _inYaxis = !(_voxel.mBBY < _colliderbb.y() || _voxel.mbbY > _colliderBB.y());
//    bool _inZaxis = !(_voxel.mBBZ < _colliderbb.z() || _voxel.mbbZ > _colliderBB.z());

//    return _inXaxis && _inYaxis && _inZaxis;
}

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
                _voxels.push_back(_linearCoord);
            }
    return _voxels;
}
