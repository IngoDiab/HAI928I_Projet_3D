#include "ShaderProgram.h"
#include <QFile>
#include <QDebug>

#include <QOpenGLExtraFunctions>

ShaderProgram::ShaderProgram()
{

}

ShaderProgram::ShaderProgram(const QString& _vPath, const QString& _fPath)
{
    mType = SHADER_TYPE::RENDERING;
    BindProgram();
    AddShader(_vPath, QOpenGLShader::Vertex);
    AddShader(_fPath, QOpenGLShader::Fragment);
    LinkShaders();

    GetLocation(mModelMatrixLocation, "model_matrix");
    GetLocation(mViewMatrixLocation, "view_matrix");
    GetLocation(mProjectionMatrixLocation, "projection_matrix");
}

ShaderProgram::ShaderProgram(const QString& _cPath)
{
    mType = SHADER_TYPE::COMPUTING;
    BindProgram();
    AddShader(_cPath, QOpenGLShader::Compute);
    LinkShaders();

    GetLocation(mDeltaTimeLocation, "deltaTime");
    GetLocation(mNbParticuleLocation, "nbParticles");
    GetLocation(mNbCubeCollidersLocation, "nbCubeColliders");
    GetLocation(mGridSizeLocation, "sizeGrid");
    GetLocation(mStepLocation[0], "stepX");
    GetLocation(mStepLocation[1], "stepY");
    GetLocation(mStepLocation[2], "stepZ");
    GetLocation(mbbLocation[0], "bbX");
    GetLocation(mbbLocation[1], "bbY");
    GetLocation(mbbLocation[2], "bbZ");
}



void ShaderProgram::BindProgram()
{
    mProgram.bind();
}

void ShaderProgram::UnbindProgram()
{
    mProgram.release();
}

void ShaderProgram::AddShader(const QString& _path, const QOpenGLShader::ShaderTypeBit& _type)
{
    QString _shaderCode = LoadShaderFromPath(_path);
    if(_shaderCode == "") return;

    QOpenGLShader* _shader = new QOpenGLShader(_type);
    if (!_shader->compileSourceCode(_shaderCode)) return;
    //qWarning() << "Ce shader a bien ete compile : " << _path;
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

void ShaderProgram::ProcessComputeShader(unsigned int _numGroupsX, unsigned int _numGroupsY, unsigned int _numGroupsZ)
{
    if(mType != SHADER_TYPE::COMPUTING) return;
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glDispatchCompute(_numGroupsX, _numGroupsY, _numGroupsZ);
    _functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    _functions.glFinish();
}

void ShaderProgram::SendMVP(const GLfloat* _modelMatrix, const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix)
{
    BindProgram();
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, &_modelMatrix[0]);
    _functions.glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, &_viewMatrix[0]);
    _functions.glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, &_projectionMatrix[0]);
}

void ShaderProgram::SendM(const GLfloat* _modelMatrix)
{
    BindProgram();
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniformMatrix4fv(mModelMatrixLocation, 1, GL_FALSE, &_modelMatrix[0]);
}

void ShaderProgram::SendVP(const GLfloat* _viewMatrix, const GLfloat* _projectionMatrix)
{
    BindProgram();
    QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
    _functions.glUniformMatrix4fv(mViewMatrixLocation, 1, GL_FALSE, &_viewMatrix[0]);
    _functions.glUniformMatrix4fv(mProjectionMatrixLocation, 1, GL_FALSE, &_projectionMatrix[0]);
}

void ShaderProgram::GetLocation(int& _location, string _name)
{
   _location = mProgram.uniformLocation(_name.c_str());
}
