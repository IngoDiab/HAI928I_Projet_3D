#ifndef VBO_H
#define VBO_H

#include <QOpenGLFunctions>
#include <vector>

#include <QOpenGLBuffer>
using namespace std;


#include <QDebug>

class VBO : protected QOpenGLFunctions
{
    unsigned int mBufferType;
    unsigned int mBufferHandle;
    //QOpenGLBuffer mBuffer;

   public:
       VBO();
       VBO(const unsigned int _bufferType); //QOpenGLBuffer::Type _bufferType);
       virtual ~VBO();

   public:
       void GenerateVBO();
       void DeleteVBO();

       void BindVBO();

       template <typename BufferData>
       void CopyDataToVBO(const QVector<BufferData>& _data);

       void DrawVBO(const unsigned int _attribute, const int _size, const unsigned int _type, const unsigned char _normalized = 0, const int _stride = 0, const int _offsetBegin = 0);
       void DrawVBOIndices(const unsigned int _mode, const int _size, const int _offsetBegin = 0);

       //VBO& operator= (const VBO&& _other);
};

template <typename BufferData>
void VBO::CopyDataToVBO(const QVector<BufferData>& _data)
{
    BindVBO();
//    mBuffer.allocate(&_data[0], _data.size() * sizeof(BufferData));
//    float* data[6];
//    mBuffer.read(0, &data[0], sizeof(float)*6);
//    qDebug() << _data.size();
//    qDebug() << data[0] << " " << data[1] << " " <<data[2] << " " <<data[3] << " " << data[4] << " " << data[5] << " ";
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glBufferData(mBufferType, _data.size() * sizeof(BufferData), &_data[0] , 0x88E4); //GL_STATIC_DRAW
}

#endif // VBO_H
