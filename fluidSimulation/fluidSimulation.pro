TEMPLATE = app
TARGET = myProject
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./src
QT += opengl xml core gui widgets
CONFIG += qt
MOC_DIR = ./tmp/moc
OBJECTS_DIR = ./tmp/obj

# Input
HEADERS += ./src/Mesh/Mesh.h \
    ./src/others/MyViewer.h \
    ./src/gl/GLUtilityMethods.h \
    ./src/gl/BasicColors.h \
    src/Utils/Singleton.h \
    src/Container/Container.h \
    src/Grid/Grid.h \
    src/Mesh/Cube/Cube.h \
    src/Physics/Colliders/Cube/CubeCollider.h \
    src/Physics/PhysicManager/PhysicManager.h \
    src/Transform/Transform.h \
    src/Fluid/Fluid.h \
    src/Particle/Particle.h \
    src/Mesh/MyMesh.h \
    src/Mesh/Sphere/Sphere.h \
    src/ShaderProgram/ShaderProgram.h \
    src/Buffer/Buffer.h

SOURCES += ./src/main.cpp \
    ./src/gl/GLUtilityMethods.cpp\
    ./src/gl/BasicColors.cpp \
    src/Container/Container.cpp \
    src/Grid/Grid.cpp \
    src/Mesh/Cube/Cube.cpp \
    src/Physics/Colliders/Cube/CubeCollider.cpp \
    src/Physics/PhysicManager/PhysicManager.cpp \
    src/Transform/Transform.cpp \
    src/Fluid/Fluid.cpp \
    src/Particle/Particle.cpp \
    src/Mesh/MyMesh.cpp \
    src/Mesh/Sphere/Sphere.cpp \
    src/ShaderProgram/ShaderProgram.cpp \
    src/Buffer/Buffer.cpp

RESOURCES += \
    shaders.qrc


EXT_DIR = ../extern

INCLUDE_DIR = $${EXT_DIR}
INCLUDEPATH += $${INCLUDE_DIR}/include
DEPENDPATH  += $${INCLUDE_DIR}/include

LIB_DIR = $${EXT_DIR}/QGLViewer

LIBS += -lopengl32 -lglu32 -lgomp
LIBS += -L$${LIB_DIR}

isEmpty( QGLVIEWER_STATIC ) {
        CONFIG(debug, debug|release) {
                LIBS += -L$${LIB_DIR}/bin -lQGLViewerd2
        } else {
                LIBS += -L$${LIB_DIR}/bin -lQGLViewer2
        }
} else {
        DEFINES *= QGLVIEWER_STATIC
        CONFIG(debug, debug|release) {
                LIBS += $${LIB_DIR}/lib/libQGLViewerd2.a
        } else {
                LIBS += $${LIB_DIR}/lib/libQGLViewer2.a
        }
}

release:QMAKE_CXXFLAGS_RELEASE += -O3 \
    -fopenmp
release:QMAKE_CFLAGS_RELEASE += -O3 \
    -fopenmp
