#ifndef MYMESH_H
#define MYMESH_H


#include <vector>
#include <string>
using namespace std;

#include <QVector3D>
#include "../VBO/VBO.h"

enum class VERTEX_ATTRIBUTE
{
    VERTEX_POSITION = 0,
    VERTEX_UVS = 1,
    VERTEX_NORMALE = 2,
    VERTEX_INDICES = 3
};

struct Triangle
{
    vector<unsigned int> mIndices = vector<unsigned int>(3);

    Triangle(const vector<unsigned int>& _indices) : mIndices(_indices){}
    Triangle(const unsigned int _indice0, const unsigned int _indice1, const unsigned int _indice2)
    {
        mIndices[0] = _indice0;
        mIndices[1] = _indice1;
        mIndices[2] = _indice2;
    }

    unsigned int operator[](const int _index) { return mIndices[_index]; }
};

class MyMesh
{
friend class MyViewer;

protected:
    QVector<QVector3D> mPositions = QVector<QVector3D>();
    VBO mPositionVBO = VBO(GL_ARRAY_BUFFER);

//    vector<point2d> mUVs = vector<vec2>();
//    VBO mUVsVBO = VBO(GL_ARRAY_BUFFER);

    QVector<QVector3D> mNormales = QVector<QVector3D>();
    VBO mNormalVBO = VBO(GL_ARRAY_BUFFER);

    QVector<unsigned int> mIndices = QVector<unsigned int>();
    VBO mIndicesVBO = VBO(GL_ELEMENT_ARRAY_BUFFER);

    QVector<Triangle> mTriangles;

public:
    QVector<QVector3D> GetPositionsVertices() const {return mPositions;}
    QVector<QVector3D> GetNormalesVertices() const {return mNormales;}
    QVector<unsigned int> GetIndices() const {return mIndices;}
    void SetPositionsVertices(const QVector<QVector3D>& _positions) {mPositions = _positions; RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_POSITION);}

public:
    MyMesh();
    virtual ~MyMesh();
    MyMesh(const QVector<QVector3D>& _positions, /*const vector<vec2>& _uvs,*/ const QVector<unsigned int>& _indices, const QVector<Triangle>& _triangles);

protected:
    void RefreshVBOData(const VERTEX_ATTRIBUTE _vbo);

    void ClearMeshBuffers();
    virtual void CreateVerticesPositions();
    virtual void CreateVerticesUVs();
    virtual void CreateIndices();
    virtual void CreateVerticesNormales();

public:
    void DrawMesh();
};
#endif // MYMESH_H
