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

MyMesh::MyMesh(const QVector<QVector3D>& _positions, /*const vector<vec2>& _uvs,*/ const QVector<unsigned int>& _indices, const QVector<Triangle>& _triangles) : mPositions(_positions),/* mUVs(_uvs),*/ mIndices(_indices), mTriangles(_triangles)
{
    RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_POSITION);
    //RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_UVS);
    RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_NORMALE);
    RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_INDICES);
}

void MyMesh::RefreshVBOData(const VERTEX_ATTRIBUTE _vbo)
{
    switch (_vbo)
    {
    case VERTEX_ATTRIBUTE::VERTEX_POSITION:
        mPositionVBO.CopyDataToVBO(mPositions);
        break;

//    case VERTEX_ATTRIBUTE::VERTEX_UVS:
//        mUVsVBO.CopyDataToVBO(mUVs);
//        break;

    case VERTEX_ATTRIBUTE::VERTEX_NORMALE:
        mNormalVBO.CopyDataToVBO(mNormales);
        break;

    case VERTEX_ATTRIBUTE::VERTEX_INDICES:
        mIndicesVBO.CopyDataToVBO(mIndices);
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
    mPositionVBO.DrawVBO(0, 3, GL_FLOAT);
    //mUVsVBO.DrawVBO(1, 2, GL_FLOAT);
    mNormalVBO.DrawVBO(1, 3, GL_FLOAT);
    mIndicesVBO.DrawVBOIndices(GL_TRIANGLES, mIndices.size());
}
