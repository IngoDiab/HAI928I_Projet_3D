#include "CubeCollider.h"
#include <QMatrix4x4>
#include "../../../Transform/Transform.h"
#include "../../PhysicManager/PhysicManager.h"

CubeCollider::CubeCollider(const Transform* _transform) : mParent(_transform)
{
    RefreshColliderTransform();
    PhysicManager::Instance()->AddCollider(this);
}

void CubeCollider::RefreshColliderTransform()
{
    QMatrix4x4 _modelMatrix = mParent->GetModelMatrix();

    mPositions = QVector<QVector3D>() =
        {
#pragma region Face Bottom
            _modelMatrix * QVector3D(-0.5,-0.5,-0.5), //LeftBotFar 0
            _modelMatrix *QVector3D(0.5,-0.5,-0.5), //RightBotFar 1
            _modelMatrix *QVector3D(-0.5,-0.5,0.5), //LeftBotNear 2
            _modelMatrix *QVector3D(0.5,-0.5,0.5), //RightBotNear 3
#pragma endregion

#pragma region Face Top
            _modelMatrix *QVector3D(0.5,0.5,-0.5), //RightTopFar 4
            _modelMatrix *QVector3D(-0.5,0.5,-0.5), //LeftTopFar 5
            _modelMatrix *QVector3D(0.5,0.5,0.5), //RightTopNear 6
            _modelMatrix *QVector3D(-0.5,0.5,0.5), //LeftTopNear 7
#pragma endregion
        };
}

void CubeCollider::DrawFace(const unsigned int& _index1, const unsigned int& _index2, const unsigned int& _index3) const
{
    glColor3f(0,0.9f,0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(mPositions[_index1].x(), mPositions[_index1].y(), mPositions[_index1].z());
    glVertex3f(mPositions[_index2].x(), mPositions[_index2].y(), mPositions[_index2].z());
    glVertex3f(mPositions[_index3].x(), mPositions[_index3].y(), mPositions[_index3].z());
    glEnd();
}

void CubeCollider::Render() const
{
    glDisable(GL_LIGHTING);
    DrawFace(0,1,3); //Bottom
    DrawFace(0,3,2); //Bottom

    DrawFace(7,6,4); //Top
    DrawFace(7,4,5); //Top

    DrawFace(1,0,5); //Back
    DrawFace(1,5,4); //Back

    DrawFace(2,3,6); //Front
    DrawFace(2,6,7); //Front

    DrawFace(0,2,7); //Left
    DrawFace(0,7,5); //Left

    DrawFace(3,1,4); //Right
    DrawFace(3,4,6); //Right

    glLineWidth(12);
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex3f(mPositions[0].x(), mPositions[0].y(), mPositions[0].z());
    glVertex3f((mPositions[0]+GetXAxisCollision()).x(), (mPositions[0]+GetXAxisCollision()).y(), (mPositions[0]+GetXAxisCollision()).z());
    glEnd();
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(mPositions[0].x(), mPositions[0].y(), mPositions[0].z());
    glVertex3f((mPositions[0]+GetYAxisCollision()).x(), (mPositions[0]+GetYAxisCollision()).y(), (mPositions[0]+GetYAxisCollision()).z());
    glEnd();
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(mPositions[0].x(), mPositions[0].y(), mPositions[0].z());
    glVertex3f((mPositions[0]+GetZAxisCollision()).x(), (mPositions[0]+GetZAxisCollision()).y(), (mPositions[0]+GetZAxisCollision()).z());
    glEnd();
    glLineWidth(1);

    glEnable(GL_LIGHTING);
}
