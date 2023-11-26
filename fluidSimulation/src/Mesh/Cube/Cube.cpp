#include "Cube.h"

Cube::Cube() : MyMesh()
{
    CreateCube();
}

Cube::Cube(const bool _generatePos, const bool _generateUV, const bool _generateIndices, const bool _generateNormales) : MyMesh()
{
    CreateCube(_generatePos, _generateUV, _generateIndices, _generateNormales);
}

QVector<QVector3D> Cube::ModelPositions() const
{
    return QVector<QVector3D>() =
           {
#pragma region Face Bottom
               QVector3D(-0.5,-0.5,-0.5), //LeftBotFar
               QVector3D(0.5,-0.5,-0.5), //RightBotFar
               QVector3D(-0.5,-0.5,0.5), //LeftBotNear
               QVector3D(0.5,-0.5,0.5), //RightBotNear
#pragma endregion

#pragma region Face Top
               QVector3D(0.5,0.5,-0.5), //RightTopFar
               QVector3D(-0.5,0.5,-0.5), //LeftTopFar
               QVector3D(0.5,0.5,0.5), //RightTopNear
               QVector3D(-0.5,0.5,0.5), //LeftTopNear
#pragma endregion
           };
}

void Cube::CreateCube(const bool _generatePos, const bool _generateUV, const bool _generateIndices, const bool _generateNormales)
{
    if(_generatePos)
    {
        if(_generateUV) CreateVerticesPositions();
        else CreateVerticesPositionsPacked();
        RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_POSITION);
    }

    if(_generateUV)
    {
        CreateVerticesUVs();
        RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_UVS);
    }

    if(_generateIndices)
    {
        if(_generateUV) CreateIndices();
        else CreateIndicesPacked();
        RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_INDICES);
    }

    if(_generateNormales)
    {
        CreateVerticesNormales();
        RefreshVBOData(VERTEX_ATTRIBUTE::VERTEX_NORMALE);
    }
}

void Cube::CreateVerticesPositions()
{
    mPositions.clear();
    mPositions =
        {
#pragma region Face Bottom
            QVector3D(-0.5,-0.5,-0.5), //LeftBotFar
            QVector3D(0.5,-0.5,-0.5), //RightBotFar
            QVector3D(-0.5,-0.5,0.5), //LeftBotNear
            QVector3D(0.5,-0.5,0.5), //RightBotNear
#pragma endregion

#pragma region Face Top
            QVector3D(0.5,0.5,-0.5), //RightTopFar
            QVector3D(-0.5,0.5,-0.5), //LeftTopFar
            QVector3D(0.5,0.5,0.5), //RightTopNear
            QVector3D(-0.5,0.5,0.5), //LeftTopNear
#pragma endregion

#pragma region Face Front
            QVector3D(-0.5,-0.5,0.5), //LeftBotNear
            QVector3D(0.5,-0.5,0.5), //RightBotNear
            QVector3D(-0.5,0.5,0.5), //LeftTopNear
            QVector3D(0.5,0.5,0.5), //RightTopNear
#pragma endregion

#pragma region Face Back
            QVector3D(0.5,0.5,-0.5), //RightTopFar
            QVector3D(-0.5,-0.5,-0.5), //LeftBotFar
            QVector3D(-0.5,0.5,-0.5), //LeftTopFar
            QVector3D(0.5,-0.5,-0.5), //RightBotFar
#pragma endregion

#pragma region Face Right
            QVector3D(0.5,0.5,-0.5), //RightTopFar
            QVector3D(0.5,-0.5,0.5), //RightBotNear
            QVector3D(0.5,-0.5,-0.5), //RightBotFar
            QVector3D(0.5,0.5,0.5), //RightTopNear
#pragma endregion

#pragma region Face Left
            QVector3D(-0.5,0.5,-0.5), //LeftTopFar
            QVector3D(-0.5,-0.5,-0.5), //LeftBotFar
            QVector3D(-0.5,-0.5,0.5), //LeftBotNear
            QVector3D(-0.5,0.5,0.5), //LeftTopNear
#pragma endregion
        };
}

void Cube::CreateVerticesPositionsPacked()
{
    mPositions.clear();
    mPositions =
        {
#pragma region Face Bottom
            QVector3D(-0.5,-0.5,-0.5), //LeftBotFar
            QVector3D(0.5,-0.5,-0.5), //RightBotFar
            QVector3D(-0.5,-0.5,0.5), //LeftBotNear
            QVector3D(0.5,-0.5,0.5), //RightBotNear
#pragma endregion

#pragma region Face Top
            QVector3D(0.5,0.5,-0.5), //RightTopFar
            QVector3D(-0.5,0.5,-0.5), //LeftTopFar
            QVector3D(0.5,0.5,0.5), //RightTopNear
            QVector3D(-0.5,0.5,0.5), //LeftTopNear
#pragma endregion
        };
}

void Cube::CreateVerticesUVs()
{
//    mUVs.clear();
//    mUVs =
//        {
//#pragma region Face Bottom
//            vec2(0,0), //LeftBotFar
//            vec2(0.5,0), //RightBotFar
//            vec2(0,0.5), //LeftBotNear
//            vec2(0.5,0.5), //RightBotNear
//#pragma endregion

//#pragma region Face Top
//            vec2(0.5,0.5), //RightTopFar
//            vec2(0,0.5), //LeftTopFar
//            vec2(0.5,0), //RightTopNear
//            vec2(0,0), //LeftTopNear
//#pragma endregion

//#pragma region Face Front
//            vec2(0,0), //LeftBotNear
//            vec2(0.5,0), //RightBotNear
//            vec2(0,0.5), //LeftTopNear
//            vec2(0.5,0.5), //RightTopNear
//#pragma endregion

//#pragma region Face Back
//            vec2(0,0.5), //RightTopFar
//            vec2(0.5,0), //LeftBotFar
//            vec2(0.5,0.5), //LeftTopFar
//            vec2(0,0), //RightBotFar
//#pragma endregion

//#pragma region Face Right
//            vec2(0.5,0.5), //RightTopFar
//            vec2(0,0), //RightBotNear
//            vec2(0.5,0), //RightBotFar
//            vec2(0,0.5), //RightTopNear
//#pragma endregion

//#pragma region Face Left
//            vec2(0.5,0.5), //LeftTopFar
//            vec2(0.5,0), //LeftBotFar
//            vec2(0,0), //LeftBotNear
//            vec2(0,0.5) //LeftTopNear
//#pragma endregion
//        };

}

void Cube::CreateIndices()
{
    mIndices.clear();
    mIndices =
        {
#pragma region Face Bottom
            2, 1, 0,
            2, 3, 1,
#pragma endregion

#pragma region Face Top
            6, 5, 4,
            6, 7, 5,
#pragma endregion

#pragma region Face Front
            10, 9, 8,
            10, 11, 9,
#pragma endregion

#pragma region Face Back
            14, 13, 12,
            13, 15, 12,
#pragma endregion

#pragma region Face Right
            18, 17, 16,
            17, 19, 16,
#pragma endregion

#pragma region Face Left
            22, 21, 20,
            23, 22, 20,
#pragma endregion
        };
}

void Cube::CreateIndicesPacked()
{
    mIndices.clear();
    mIndices =
        {
#pragma region Face Bottom
            2, 1, 0,
            2, 3, 1,
#pragma endregion

#pragma region Face Top
            6, 5, 4,
            6, 7, 5,
#pragma endregion

#pragma region Face Front
            7, 3, 2,
            7, 6, 3,
#pragma endregion

#pragma region Face Back
            5, 0, 4,
            0, 1, 4,
#pragma endregion

#pragma region Face Right
            1, 3, 4,
            3, 6, 4,
#pragma endregion

#pragma region Face Left
            2, 0, 5,
            7, 2, 5,
#pragma endregion
        };
}

void Cube::CreateVerticesNormales()
{
    mNormales.clear();
    mNormales.resize(mPositions.size());

    QVector<unsigned int> _neighboors = QVector<unsigned int>(mPositions.size());
    unsigned int _nbIndices = mIndices.size();
    for(int i = 0; i < _nbIndices; i+=3)
    {
        //Get Indices
        unsigned int _index0 = mIndices[i];
        unsigned int _index1 = mIndices[i+1];
        unsigned int _index2 = mIndices[i+2];

        //Increm. neighboors
        ++_neighboors[_index0];
        ++_neighboors[_index1];
        ++_neighboors[_index2];

        //Normale triangle
        QVector3D _pos0 = mPositions[_index0];
        QVector3D _pos1 = mPositions[_index1];
        QVector3D _pos2 = mPositions[_index2];
        QVector3D _triangleNormales = (QVector3D::crossProduct(_pos2-_pos0, _pos1-_pos0)).normalized();

        //Add normal triangle
        mNormales[_index0] += _triangleNormales;
        mNormales[_index1] += _triangleNormales;
        mNormales[_index2] += _triangleNormales;
    }

    //Get Normale per vertex
    unsigned int _nbVertex = mNormales.size();
    for(int i = 0; i < _nbVertex; ++i)
    {
        mNormales[i] /= (float)_neighboors[i];
        mNormales[i] = mNormales[i].normalized();
    }
}
