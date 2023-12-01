#ifndef OGL_BUFFER_H
#define OGL_BUFFER_H

#include <QOpenGLExtraFunctions>
#include <vector>

#include <QOpenGLBuffer>
using namespace std;


#include <QDebug>

class OGL_Buffer : protected QOpenGLExtraFunctions
{
    unsigned int mBufferType;
//    unsigned int mBufferHandle;
    QOpenGLBuffer mBuffer;

   public:
       OGL_Buffer();
       OGL_Buffer(QOpenGLBuffer::Type _type);
       virtual ~OGL_Buffer();

   public:
       void GenerateBuffer();
       void DeleteBuffer();

       void BindBuffer();

       template <typename BufferData>
       void CopyDataToBuffer(const QVector<BufferData>& _data);
       template <typename BufferData>
       BufferData* RetrieveFromComputeShader();

       void DrawBuffer(const unsigned int _attribute, const int _size, const unsigned int _type, const unsigned char _normalized = 0, const int _stride = 0, const int _offsetBegin = 0);
       void DrawBufferIndices(const unsigned int _mode, const int _size, const int _offsetBegin = 0);

       void BindBase(int _index);
       void SetUsagePattern(QOpenGLBuffer::UsagePattern _pattern);
       void* Map(QOpenGLBuffer::Access _access);
       void Unmap();
};

template <typename BufferData>
void OGL_Buffer::CopyDataToBuffer(const QVector<BufferData>& _data)
{
    BindBuffer();
    mBuffer.allocate(&_data[0], _data.size() * sizeof(BufferData));
}

template <typename BufferData>
BufferData* OGL_Buffer::RetrieveFromComputeShader()
{
    BufferData* _retrievedData = static_cast<BufferData*>(Map(QOpenGLBuffer::ReadOnly));
    Unmap();
    return _retrievedData;
}
#endif // OGL_BUFFER_H
