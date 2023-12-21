#include "MyMesh.h"

MyMesh::MyMesh()
{
//    mPositionVBO = VBO(GL_ARRAY_BUFFER);
//    mNormalVBO = VBO(GL_ARRAY_BUFFER);
//    mIndicesVBO = VBO(GL_ELEMENT_ARRAY_BUFFER);
}

MyMesh::~MyMesh()
{
//    delete mPositionVBO;
//    delete mNormalVBO;
//    delete mIndicesVBO;
}

MyMesh::MyMesh(const QVector<QVector3D>& _positions, /*const vector<vec2>& _uvs,*/  const QVector<QVector3D>& _normales, const QVector<unsigned int>& _indices) : mPositions(_positions),/* mUVs(_uvs),*/ mNormales(_normales), mIndices(_indices)
{
    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_POSITION);
    //RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_UVS);
    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_NORMALE);
    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_INDICES);
}

void MyMesh::RefreshMesh(const QVector<QVector3D>& _positions,  const QVector<QVector3D>& _normales, const QVector<unsigned int>& _indices)
{
    mPositions = _positions;
    mNormales = _normales;
    mIndices = _indices;

    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_POSITION);
    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_NORMALE);
    RefreshBufferData(VERTEX_ATTRIBUTE::VERTEX_INDICES);
}

void MyMesh::RefreshBufferData(const VERTEX_ATTRIBUTE _vbo)
{
    switch (_vbo)
    {
    case VERTEX_ATTRIBUTE::VERTEX_POSITION:
        mPositionVBO.CopyDataToBuffer(mPositions);
        break;

//    case VERTEX_ATTRIBUTE::VERTEX_UVS:
//        mUVsVBO.CopyDataToBuffer(mUVs);
//        break;

    case VERTEX_ATTRIBUTE::VERTEX_NORMALE:
        mNormalVBO.CopyDataToBuffer(mNormales);
        break;

    case VERTEX_ATTRIBUTE::VERTEX_INDICES:
        mIndicesVBO.CopyDataToBuffer(mIndices);
        break;

    default:
        break;
    }
}

void MyMesh::ClearMeshBuffers()
{
    mPositions.clear();
    //mUVs.clear();
    mNormales.clear();
    mIndices.clear();
}

void MyMesh::CreateVerticesPositions()
{

}

void MyMesh::CreateVerticesUVs()
{

}

void MyMesh::CreateIndices()
{

}


void MyMesh::CreateVerticesNormales()
{

}

void MyMesh::DrawMesh()
{
    if(mPositions.size() == 0) return;
    mPositionVBO.UseBuffer(0, 3, GL_FLOAT);
    mNormalVBO.UseBuffer(1, 3, GL_FLOAT);
    if(mIndices.size() != 0) mIndicesVBO.DrawBufferIndices(GL_TRIANGLES, mIndices.size());
    else
    {
        mPositionVBO.BindBuffer();
        mPositionVBO.DrawBuffer(GL_TRIANGLES, 0, mPositions.size());
    }
}
