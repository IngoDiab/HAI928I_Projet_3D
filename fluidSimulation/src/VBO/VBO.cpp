#include "VBO.h"
#include <QOpenGLFunctions>

VBO::VBO()
{

}

VBO::VBO(const QOpenGLBuffer::Type _bufferType)
{
    mBuffer = QOpenGLBuffer(_bufferType);
    GenerateVBO();
}

VBO::~VBO()
{
    DeleteVBO();
}

void VBO::GenerateVBO()
{
    mBuffer.create();
    //glGenBuffers(1, &mBufferHandle);
}

void VBO::DeleteVBO()
{
    mBuffer.destroy();
    //glDeleteBuffers(1, &mBufferHandle);
}

void VBO::BindVBO()
{
    mBuffer.bind();
    //glBindBuffer(mBufferType, mBufferHandle);
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
    BindVBO();
    glDrawElements(_mode, _size, GL_UNSIGNED_INT, (void*)_offsetBegin);
}
