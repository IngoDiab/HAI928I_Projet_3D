TEMPLATE = app
TARGET = myProject
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./src
QT += opengl xml core gui
CONFIG += qt
MOC_DIR = ./tmp/moc
OBJECTS_DIR = ./tmp/obj

# Input
HEADERS += ./src/point3.h \
    ./src/Mesh.h \
    ./src/MyViewer.h \
    ./src/gl/GLUtilityMethods.h \
    ./src/gl/BasicColors.h
SOURCES += ./src/main.cpp \
    ./src/gl/GLUtilityMethods.cpp\
    ./src/gl/BasicColors.cpp



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
