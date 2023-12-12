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
       bool ReadFromBuffer(int _offset, BufferData* _dataOut, int _nbBytes);

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
    if(_data.size() <= 0) return;
    BindBuffer();
    mBuffer.allocate(&_data[0], _data.size() * sizeof(BufferData));
}

template <typename BufferData>
bool OGL_Buffer::ReadFromBuffer(int _offset, BufferData* _dataOut, int _nbBytes)
{
    BindBuffer();
    return mBuffer.read(_offset, _dataOut, _nbBytes);
}
#endif // OGL_BUFFER_H
