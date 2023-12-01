#include "Buffer.h"
#include <GL/glext.h>

OGL_Buffer::OGL_Buffer()
{
    GenerateBuffer();
}

OGL_Buffer::OGL_Buffer(QOpenGLBuffer::Type _type)//GLenum _bufferType) : mBufferType(_bufferType)
{
    mBuffer = QOpenGLBuffer(_type);
    GenerateBuffer();
}

OGL_Buffer::~OGL_Buffer()
{
    DeleteBuffer();
}

void OGL_Buffer::GenerateBuffer()
{
    mBuffer.create();
//    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
//    _functions.glGenBuffers(1, &mBufferHandle);
}

void OGL_Buffer::DeleteBuffer()
{
    mBuffer.destroy();
//    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
//    _functions.glDeleteBuffers(1, &mBufferHandle);
}

void OGL_Buffer::BindBuffer()
{
    mBuffer.bind();
//    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
//    _functions.glBindBuffer(mBufferType, mBufferHandle);
}

void OGL_Buffer::DrawBuffer(const GLuint _attribute, const GLint _size, const GLenum _type, const GLboolean _normalized, const GLsizei _stride, const int _offsetBegin)
{
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glEnableVertexAttribArray(_attribute);
    BindBuffer();
    _functions.glVertexAttribPointer(_attribute, _size, _type, _normalized, _stride, (void*)_offsetBegin);
}

void OGL_Buffer::DrawBufferIndices(const GLenum _mode, const GLsizei _size, const int _offsetBegin)
{
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    BindBuffer();
    _functions.glDrawElements(_mode, _size, GL_UNSIGNED_INT, (void*)_offsetBegin);
}

void OGL_Buffer::BindBase(int _index)
{
    BindBuffer();
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _index, mBuffer.bufferId());
}

void OGL_Buffer::SetUsagePattern(QOpenGLBuffer::UsagePattern _pattern)
{
    mBuffer.setUsagePattern(_pattern);
}

void* OGL_Buffer::Map(QOpenGLBuffer::Access _access)
{
    BindBuffer();
    return mBuffer.map(_access);
}

void OGL_Buffer::Unmap()
{
    BindBuffer();
    mBuffer.unmap();
}

