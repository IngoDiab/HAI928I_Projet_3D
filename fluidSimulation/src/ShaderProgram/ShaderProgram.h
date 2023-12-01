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

    int mNbParticuleLocation;
    int mGridSizeLocation;

public:
    SHADER_TYPE GetType() const {return mType;}
    int GetNbParticuleLocation() const {return mNbParticuleLocation;}
    int GetGridSizeLocation() const {return mGridSizeLocation;}

public:
    ShaderProgram();
    ShaderProgram(const QString& _vPath, const QString& _fPath);
    ShaderProgram(const QString& _cPath);

public:
    void BindProgram();
    void UnbindProgram();
    void LinkShaders();
    void ProcessComputeShader(unsigned int _numGroupsX, unsigned int _numGroupsY, unsigned int _numGroupsZ);

    void SendMVP(const GLfloat* _modelMatrix, const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);
    void SendM(const GLfloat* _modelMatrix);
    void SendVP(const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);

private:
    void AddShader(const QString& _path, const QOpenGLShader::ShaderTypeBit& _type);
    QString LoadShaderFromPath(const QString& _path);
    void GetLocation(int& _location, string _name);
};

#endif // SHADERPROGRAM_H
