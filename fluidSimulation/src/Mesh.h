#ifndef PROJECTMESH_H
#define PROJECTMESH_H

#include <vector>
#include <QVector3D>

struct Vertex{
    QVector3D p;
    Vertex() {}
    Vertex(double x , double y , double z) : p(x,y,z) {}
};

struct Triangle{
    unsigned int corners[3];
    unsigned int & operator [] (unsigned int c) { return corners[c]; }
    unsigned int operator [] (unsigned int c) const { return corners[c]; }
    unsigned int size() const { return 3 ; }
};

struct Mesh{
    std::vector< Vertex > vertices;
    std::vector< Triangle > triangles;
};



#endif // PROJECTMESH_H
