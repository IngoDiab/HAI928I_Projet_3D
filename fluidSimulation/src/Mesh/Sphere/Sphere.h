#ifndef SPHERE_H
#define SPHERE_H

#include "../MyMesh.h"

class Sphere : public MyMesh
{
    int mNbVertexAzimut = 10;
    int mNbVertexElevation = 10;

    public:
        Sphere();
        Sphere(const bool _generatePos, const bool _generateUV, const bool _generateIndices, const bool _generateNormales);
        Sphere(const int _nbVertexAzimut, const int _nbVertexElevation);

    protected:
        void CreateSphere(const bool _generatePos = true, const bool _generateUV = true, const bool _generateIndices = true, const bool _generateNormales = true);
        virtual void CreateVerticesPositions() override;
        virtual void CreateVerticesUVs() override;
        virtual void CreateIndices() override;
        virtual void CreateVerticesNormales() override;

    private:
        QVector3D SphericalCoordinatesToEuclidean( float theta , float phi );
};

#endif // SPHERE_H
