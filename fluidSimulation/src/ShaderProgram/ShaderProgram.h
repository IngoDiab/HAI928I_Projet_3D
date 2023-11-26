#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <QString>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <string>
using namespace std;

class ShaderProgram
{
    QOpenGLShaderProgram mProgram;

    int mModelMatrixLocation;
    int mViewMatrixLocation;
    int mProjectionMatrixLocation;

public:
    ShaderProgram();
    ShaderProgram(const QString& _vPath, const QString& _fPath);

public:
    void BindProgram();
    void LinkShaders();

    void SendMVP(const GLfloat* _modelMatrix, const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);
    void SendM(const GLfloat* _modelMatrix);
    void SendVP(const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix);

private:
    void AddShader(const QString& _path, const QOpenGLShader::ShaderTypeBit& _type);
    QString LoadShaderFromPath(const QString& _path);
    void GetLocation(int& _location, string _name);
};

#endif // SHADERPROGRAM_H
