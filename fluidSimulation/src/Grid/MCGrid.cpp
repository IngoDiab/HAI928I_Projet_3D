#include "MCGrid.h"
#include "QtDebug"
#include <QGLWidget>
#include <cmath>
using namespace std;

MCGrid::MCGrid(uint _nbVoxelPerSide)
{
    mNbVoxelsPerSide = _nbVoxelPerSide;
    mNbVoxels = mNbVoxelsPerSide*mNbVoxelsPerSide*mNbVoxelsPerSide;
    mAllVoxels.resize(mNbVoxels);
}

void MCGrid::GenerateBoundingBoxGrid(const QVector3D& _bb, const QVector3D& _BB)
{
    mbb = _bb - QVector3D(mOffset, mOffset, mOffset);
    mBB = _BB + QVector3D(mOffset, mOffset, mOffset);

    mStep[0] = (mBB.x() - mbb.x())/(float)mNbVoxelsPerSide;
    mStep[1] = (mBB.y() - mbb.y())/(float)mNbVoxelsPerSide;
    mStep[2] = (mBB.z() - mbb.z())/(float)mNbVoxelsPerSide;
}

void MCGrid::DrawGrid() const
{
    if(!mDrawGrid) return;
    //qDebug() << mAllVoxels[10].mCornersDensity[0];
    for(uint i = 0; i < mNbVoxels; ++i)
        DisplayVoxel(mAllVoxels[i]);
}

void MCGrid::DisplayVoxel(const MCVoxel& _voxel) const
{
    glDisable(GL_LIGHTING);
    glPointSize(3);
    glBegin(GL_POINTS);
    for(uint i = 0; i < 8; ++i)
    {
        _voxel.mCornersDensity[i] < mSeuil ? glColor3f(0,0,0) : glColor3f(1,0,0);
        glVertex3f(_voxel.mCorners[i*3], _voxel.mCorners[i*3+1], _voxel.mCorners[i*3+2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}
