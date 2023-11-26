#ifndef CUBE_H
#define CUBE_H

#include "../MyMesh.h"

class Cube : public MyMesh
{
public:
    Cube();
    Cube(const bool _generatePos, const bool _generateUV, const bool _generateIndices, const bool _generateNormales);

public:
    QVector<QVector3D> ModelPositions() const;

protected:
    void CreateCube(const bool _generatePos = true, const bool _generateUV = true, const bool _generateIndices = true, const bool _generateNormales = true);
    virtual void CreateVerticesPositions() override;
    void CreateVerticesPositionsPacked();
    virtual void CreateVerticesUVs() override;
    virtual void CreateIndices() override;
    void CreateIndicesPacked();
    virtual void CreateVerticesNormales() override;
};

#endif // CUBE_H
