#include "Grid.h"
#include "QtDebug"
#include <cmath>
#include <QGLWidget>
#include "../Physics/Colliders/Cube/CubeCollider.h"
#include "../Utils/Macros.h"
using namespace std;

Grid::Grid(uint _nbVoxelPerSide)
{
    mNbVoxelsPerSide = _nbVoxelPerSide;
    mNbVoxels = mNbVoxelsPerSide*mNbVoxelsPerSide*mNbVoxelsPerSide;
    mAllVoxels.resize(mNbVoxels);
    mAllVoxelsParticlesIndices = QVector<uint>(mNbVoxels*MAX_PARTICLES_PER_VOXEL, 0);
    mAllVoxelsCubeCollidersIndices = QVector<uint>(mNbVoxels*MAX_CUBE_COLLIDERS_PER_VOXEL, 0);
}

void Grid::GenerateBoundingBoxGrid(const QVector3D& _bb, const QVector3D& _BB)
{
    mbb = _bb - QVector3D(mNbVoxelsPerSide*mOffset, mNbVoxelsPerSide*mOffset, mNbVoxelsPerSide*mOffset);
    mBB = _BB + QVector3D(mNbVoxelsPerSide*mOffset, mNbVoxelsPerSide*mOffset, mNbVoxelsPerSide*mOffset);

    mStep[0] = (mBB.x() - mbb.x())/(float)mNbVoxelsPerSide;    
    mStep[1] = (mBB.y() - mbb.y())/(float)mNbVoxelsPerSide;
    mStep[2] = (mBB.z() - mbb.z())/(float)mNbVoxelsPerSide;
}

void Grid::DrawGrid() const
{
    if(!mDrawGrid) return;
    for(uint i = 0; i < mNbVoxels; ++i)
    {
        Voxel _voxel = mAllVoxels[i];
        if(_voxel.mNbParticles >= 1) continue;
        glColor3f(0,0,0);
        DisplayVoxel(_voxel.mCorners);
    }

    for(uint i = 0; i < mNbVoxels; ++i)
    {
        Voxel _voxel = mAllVoxels[i];
        if(_voxel.mNbParticles < 1) continue;
        glColor3f(1,0,0);
        DisplayVoxel(_voxel.mCorners);
    }
}

void Grid::DisplayVoxel(const float* _corners) const
{
    DrawFace(0,1,3,2, _corners); //Front
    DrawFace(4,5,7,6, _corners); //Back
    DrawFace(0,1,5,4, _corners); //Left
    DrawFace(2,3,7,6, _corners); //Right
    DrawFace(0,2,6,4, _corners); //Top
    DrawFace(1,3,7,5, _corners); //Bottom
}

void Grid::DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3, const unsigned int& _index4, const float* corners) const
{
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_LOOP);
    glVertex3f(corners[_index1*3], corners[_index1*3+1], corners[_index1*3+2]);
    glVertex3f(corners[_index2*3], corners[_index2*3+1], corners[_index2*3+2]);
    glVertex3f(corners[_index3*3], corners[_index3*3+1], corners[_index3*3+2]);
    glVertex3f(corners[_index4*3], corners[_index4*3+1], corners[_index4*3+2]);
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

void Grid::PutInVoxels(const CubeCollider& _cube, unsigned int _indexCollider)
{
    for(int i = 0; i < mAllVoxels.size(); ++i)
    {
        Voxel& _voxel = mAllVoxels[i];
        if(!IsColliderInVoxel(_voxel, i, _cube)) continue;
        mAllVoxelsCubeCollidersIndices[i*MAX_CUBE_COLLIDERS_PER_VOXEL+_voxel.mNbCubeCollider++] = _indexCollider;
    }
}

bool Grid::IsColliderInVoxel(const Voxel& _voxel, int _indexVoxel, const CubeCollider& _collider)
{
    //return false;
    QVector3D _colliderbb = _collider.Getbb();
    QVector3D _colliderBB = _collider.GetBB();

    QVector3D _voxelbb = QVector3D(_voxel.mbbX, _voxel.mbbY, _voxel.mbbZ);
    QVector3D _voxelBB = QVector3D(_voxel.mBBX, _voxel.mBBY, _voxel.mBBZ);

    //QVector<QVector3D> _cCorners = _collider.GetCorners();
    const float* _vCornersF = _voxel.mCorners;

    QVector<QVector3D> _vCorners = QVector<QVector3D>(8);
    for(int i = 0; i < 24; i+=3)
        _vCorners[i/3] = QVector3D(_vCornersF[i], _vCornersF[i+1], _vCornersF[i+2]);

    //Get relatives axis cube
    QVector3D _axis[3];
    _axis[0] = _collider.GetXAxisCollision();
    _axis[1] = _collider.GetYAxisCollision();
    _axis[2] = _collider.GetZAxisCollision();

    bool _isColliding = true;
    for(ushort i = 0; i < 3; ++i)
    {
        //Project cubeCollider
        float _cCenterProjDistancebb = QVector3D::dotProduct((_colliderbb + _colliderBB/2.0f) -_colliderbb, _axis[i]);
        QVector3D _cCenterProjPos = _colliderbb + _cCenterProjDistancebb * _axis[i];
        float _cbbProjDistancebb = QVector3D::dotProduct(_colliderbb -_colliderbb, _axis[i]);
        QVector3D _cbbProjPos = _colliderbb + _cbbProjDistancebb * _axis[i];
        float _cBBProjDistancebb = QVector3D::dotProduct(_colliderBB -_colliderbb, _axis[i]);
        QVector3D _cBBProjPos = _colliderbb + _cBBProjDistancebb * _axis[i];

        //Project voxel center
        float _vCenterProjDistancebb = QVector3D::dotProduct((_voxelbb + _voxelBB/2.0f ) -_colliderbb, _axis[i]);
        QVector3D _vCenterProjPos = _colliderbb + _vCenterProjDistancebb * _axis[i];

        float _vMinCoordOnProjAxis = _vCenterProjDistancebb, _vMaxCoordOnProjAxis = _vCenterProjDistancebb;
        QVector3D _vMinProjPos = _vCenterProjPos, _vMaxProjPos = _vCenterProjPos;

        //Each voxel corner
        for(const QVector3D& _vCorner : _vCorners)
        {
            //Project corner
            float _vCornerProjDistancebb = QVector3D::dotProduct(_vCorner -_colliderbb, _axis[i]);
            QVector3D _vCornerProjPos = _colliderbb + _vCornerProjDistancebb * _axis[i];

            if(_vCornerProjDistancebb < _vMinCoordOnProjAxis)
            {
                _vMinCoordOnProjAxis = _vCornerProjDistancebb;
                _vMinProjPos = _vCornerProjPos;
            }

            else if(_vCornerProjDistancebb > _vMaxCoordOnProjAxis)
            {
                _vMaxCoordOnProjAxis = _vCornerProjDistancebb;
                _vMaxProjPos = _vCornerProjPos;
            }
        }

//        glDisable(GL_LIGHTING);
//        glPointSize(10);
//        glBegin(GL_POINTS);
//        glColor3f(0,1,1);
//        //glVertex3f(_cbbProjPos.x(), _cbbProjPos.y(), _cbbProjPos.z());
//        //glVertex3f(_cCenterProjPos.x(),_cCenterProjPos.y(),_cCenterProjPos.z());
//        glVertex3f(_cBBProjPos.x(), _cBBProjPos.y(), _cBBProjPos.z());
//        glColor3f(1,0,0);
//        glVertex3f(_vMinProjPos.x(), _vMinProjPos.y(), _vMinProjPos.z());
//        //glVertex3f(_vCenterProjPos.x(),_vCenterProjPos.y(),_vCenterProjPos.z());
//        //glVertex3f(_vMaxProjPos.x(), _vMaxProjPos.y(), _vMaxProjPos.z());
//        glEnd();
//        glEnable(GL_LIGHTING);

        bool _vMinAftercMax = _vMinCoordOnProjAxis > _cBBProjDistancebb;
        bool _vMaxBeforecMin =  _vMaxCoordOnProjAxis < _cbbProjDistancebb;
        _isColliding &= !(_vMinAftercMax || _vMaxBeforecMin);
    }

    return _isColliding;
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
