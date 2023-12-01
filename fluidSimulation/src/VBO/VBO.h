#ifndef VBO_H
#define VBO_H

#include <QOpenGLFunctions_4_3_Core>
#include <vector>

#include <QOpenGLBuffer>
using namespace std;

class VBO : protected QOpenGLFunctions_4_3_Core
{
//    unsigned int mBufferType;
//    unsigned int mBufferHandle;
    QOpenGLBuffer mBuffer;

   public:
       VBO();
       VBO(const QOpenGLBuffer::Type _bufferType);
       virtual ~VBO();

   public:
       void GenerateVBO();
       void DeleteVBO();

       void BindVBO();

       template <typename BufferData>
       void CopyDataToVBO(const vector<BufferData>& _data);

       void DrawVBO(const unsigned int _attribute, const int _size, const unsigned int _type, const unsigned char _normalized = 0, const int _stride = 0, const int _offsetBegin = 0);
       void DrawVBOIndices(const unsigned int _mode, const int _size, const int _offsetBegin = 0);
};

template <typename BufferData>
void VBO::CopyDataToVBO(const vector<BufferData>& _data)
{
    BindVBO();
    glBufferData(mBuffer.bufferId(), _data.size() * sizeof(BufferData), &_data[0] , 0x88E4); //GL_STATIC_DRAW
}

#endif // VBO_H
