#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <QString>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLExtraFunctions>
using namespace std;

#include "../Buffer/Buffer.h"

enum class SHADER_TYPE
{
    NOT_INTIALIZED,
    RENDERING,
    COMPUTING
};

class ShaderProgram : protected QOpenGLExtraFunctions
{
    QOpenGLShaderProgram mProgram;
    SHADER_TYPE mType = SHADER_TYPE::NOT_INTIALIZED;

    int mModelMatrixLocation;
    int mViewMatrixLocation;
    int mProjectionMatrixLocation;

public:
    SHADER_TYPE GetType() const {return mType;}

public:
    ShaderProgram();
    ShaderProgram(const QString& _vPath, const QString& _fPath);
    ShaderProgram(const QString& _cPath);

public:
    void BindProgram();
    void UnbindProgram();
    void LinkShaders();
    template <typename ResultDataType>
    ResultDataType* ProcessComputeShader(unsigned int _numGroupsX, unsigned int _numGroupsY, unsigned int _numGroupsZ, OGL_Buffer& _buffer);

    void SendMVP(const GLfloat* _modelMatrix, const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);
    void SendM(const GLfloat* _modelMatrix);
    void SendVP(const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);

private:
    void AddShader(const QString& _path, const QOpenGLShader::ShaderTypeBit& _type);
    QString LoadShaderFromPath(const QString& _path);
    void GetLocation(int& _location, string _name);
};

template <typename ResultDataType>
ResultDataType* ShaderProgram::ProcessComputeShader(unsigned int _numGroupsX, unsigned int _numGroupsY, unsigned int _numGroupsZ, OGL_Buffer& _buffer)
{
    if(mType != SHADER_TYPE::COMPUTING) return nullptr;
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glDispatchCompute(_numGroupsX, _numGroupsY, _numGroupsZ);
    _functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    ResultDataType* _processedData = (ResultDataType*)_buffer.Map(QOpenGLBuffer::ReadWrite);
    _buffer.Unmap();
    return _processedData;
}

#endif // SHADERPROGRAM_H
