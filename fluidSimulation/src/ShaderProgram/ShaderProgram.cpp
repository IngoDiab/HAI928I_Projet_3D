#include "ShaderProgram.h"
#include <QFile>
#include <QDebug>

#include <QOpenGLFunctions>

ShaderProgram::ShaderProgram()
{

}

ShaderProgram::ShaderProgram(const QString& _vPath, const QString& _fPath)
{
    BindProgram();
    AddShader(_vPath, QOpenGLShader::Vertex);
    AddShader(_fPath, QOpenGLShader::Fragment);
    LinkShaders();

    GetLocation(mModelMatrixLocation, "model_matrix");
    GetLocation(mViewMatrixLocation, "view_matrix");
    GetLocation(mProjectionMatrixLocation, "projection_matrix");

    qDebug() << "mModelMatrixLocation "<< mModelMatrixLocation;
    qDebug() << "mViewMatrixLocation "<< mViewMatrixLocation;
    qDebug() << "mProjectionMatrixLocation "<< mProjectionMatrixLocation;

}

void ShaderProgram::BindProgram()
{
    mProgram.bind();
}

void ShaderProgram::AddShader(const QString& _path, const QOpenGLShader::ShaderTypeBit& _type)
{
    QString _shaderCode = LoadShaderFromPath(_path);
    if(_shaderCode == "") return;

    QOpenGLShader* _shader = new QOpenGLShader(_type);
    if (!_shader->compileSourceCode(_shaderCode)) return;
    qWarning() << "Ce shader a bien ete compile : " << _path;
    mProgram.addShader(_shader);
}

QString ShaderProgram::LoadShaderFromPath(const QString& _path)
{
    QFile _file(_path);
    if (!_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Impossible de charger le fichier :" << _path;
        return "";
    }
    QString _shaderCode = _file.readAll();
    _file.close();
    return _shaderCode;
}

void ShaderProgram::LinkShaders()
{
    mProgram.link();
}

void ShaderProgram::SendMVP(const GLfloat* _modelMatrix, const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix)
{
    BindProgram();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, &_modelMatrix[0]);
    _functions->glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, &_viewMatrix[0]);
    _functions->glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, &_projectionMatrix[0]);

    //mProgram.setUniformValue(mModelMatrixLocation, _modelMatrix);
    //mProgram.setUniformValue(mViewMatrixLocation,_viewMatrix);
    //mProgram.setUniformValue(mProjectionMatrixLocation, _projectionMatrix);
}

void ShaderProgram::SendM(const GLfloat* _modelMatrix)
{
    BindProgram();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, &_modelMatrix[0]);
    //mProgram.setUniformValue(mModelMatrixLocation, _modelMatrix);
}

void ShaderProgram::SendVP(const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix)
{
    BindProgram();
    QOpenGLFunctions* _functions = QOpenGLContext::currentContext()->functions();
    if (!_functions) return;
    _functions->glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, &_viewMatrix[0]);
    _functions->glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, &_projectionMatrix[0]);
//    mProgram.setUniformValue(mViewMatrixLocation,_viewMatrix);
//    mProgram.setUniformValue(mProjectionMatrixLocation, _projectionMatrix);
}


void ShaderProgram::GetLocation(int& _location, string _name)
{
   _location = mProgram.uniformLocation(_name.c_str());
}
