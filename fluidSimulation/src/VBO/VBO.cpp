#include "VBO.h"
#include <QOpenGLFunctions>

VBO::VBO()
{

}

VBO::VBO(const GLenum _bufferType) : mBufferType(_bufferType)
{
    //mBuffer = QOpenGLBuffer(_bufferType);
    GenerateVBO();
}

VBO::~VBO()
{
    DeleteVBO();
}

void VBO::GenerateVBO()
{
    //mBuffer.create();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glGenBuffers(1, &mBufferHandle);
}

void VBO::DeleteVBO()
{
    //mBuffer.destroy();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glDeleteBuffers(1, &mBufferHandle);
}

void VBO::BindVBO()
{
    //mBuffer.bind();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glBindBuffer(mBufferType, mBufferHandle);
}

void VBO::DrawVBO(const GLuint _attribute, const GLint _size, const GLenum _type, const GLboolean _normalized, const GLsizei _stride, const int _offsetBegin)
{
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glEnableVertexAttribArray(_attribute);
    BindVBO();
    _functions->glVertexAttribPointer(_attribute, _size, _type, _normalized, _stride, (void*)_offsetBegin);
}

void VBO::DrawVBOIndices(const GLenum _mode, const GLsizei _size, const int _offsetBegin)
{
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    BindVBO();
    _functions->glDrawElements(_mode, _size, GL_UNSIGNED_INT, (void*)_offsetBegin);
}

//VBO& VBO::operator=(const VBO&& _other)
//{
//    DeleteVBO();
//    mBufferType = _other.mBufferType;
//    mBufferHandle = _other.mBufferHandle;
//}
