#ifndef MYVIEWER_H
#define MYVIEWER_H

// Parsing:
#include "../others/BasicIO.h"

// opengl and basic gl utilities:
#define GL_GLEXT_PROTOTYPES
#include <gl/openglincludeQtComp.h>
#include <GL/glext.h>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions>
#include <QGLViewer/qglviewer.h>

#include <gl/GLUtilityMethods.h>

// Qt stuff:
#include <QFormLayout>
#include <QToolBar>
#include <QColorDialog>
#include <QFileDialog>
#include <QKeyEvent>
#include <QInputDialog>
#include <QLineEdit>


#include "qt/QSmartAction.h"

#include <vector>
#include <QOpenGLShader>
#include <QGLFunctions>

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
using namespace std;

#include <cmath>

#include "../Fluid/Fluid.h"
#include "../Container/Container.h"
#include "../Physics//Colliders/Cube/CubeCollider.h"
#include "../Physics/PhysicManager/PhysicManager.h"
#include "../Utils/Macros.h"

class MyViewer : public QGLViewer, protected QOpenGLExtraFunctions
{
    Q_OBJECT

    //Framerate
    chrono::time_point<chrono::high_resolution_clock> mPreviousFrame = chrono::high_resolution_clock::now();
    chrono::time_point<chrono::high_resolution_clock> mCurrentFrame;
    double mDeltaTime = 0;

    //Obstacles
    PhysicManager mPhysicManager;
    QVector<Container*> mContainers = QVector<Container*>(NB_CUBE_COLLIDERS, nullptr);

    //Fluid
    Fluid* mFluid = nullptr;

    //Threads
    int mMaxWorkGroupX = 0;
    int mMaxWorkGroupY = 0;
    int mMaxWorkGroupZ = 0;

    //Controls
    QWidget * controls;

public :

    MyViewer(QGLWidget * parent = NULL) : QGLViewer(parent)
    {

    }

    void CalculateDeltaTime()
    {
        mCurrentFrame = chrono::high_resolution_clock::now();
        mDeltaTime = std::chrono::duration<double>(mCurrentFrame - mPreviousFrame).count();
        mPreviousFrame = mCurrentFrame;

        qDebug() << mDeltaTime;
    }

    void add_actions_to_toolBar(QToolBar *toolBar)
    {
        // Specify the actions :
        DetailedAction * open_mesh = new DetailedAction( QIcon("./icons/open.png") , "Open Mesh" , "Open Mesh" , this , this , SLOT(open_mesh()) );
        DetailedAction * save_mesh = new DetailedAction( QIcon("./icons/save.png") , "Save model" , "Save model" , this , this , SLOT(save_mesh()) );
        DetailedAction * help = new DetailedAction( QIcon("./icons/help.png") , "HELP" , "HELP" , this , this , SLOT(help()) );
        DetailedAction * saveCamera = new DetailedAction( QIcon("./icons/camera.png") , "Save camera" , "Save camera" , this , this , SLOT(saveCamera()) );
        DetailedAction * openCamera = new DetailedAction( QIcon("./icons/open_camera.png") , "Open camera" , "Open camera" , this , this , SLOT(openCamera()) );
        DetailedAction * saveSnapShotPlusPlus = new DetailedAction( QIcon("./icons/save_snapshot.png") , "Save snapshot" , "Save snapshot" , this , this , SLOT(saveSnapShotPlusPlus()) );

        // Add them :
        toolBar->addAction( open_mesh );
        toolBar->addAction( save_mesh );
        toolBar->addAction( help );
        toolBar->addAction( saveCamera );
        toolBar->addAction( openCamera );
        toolBar->addAction( saveSnapShotPlusPlus );
    }


    void draw() {
        CalculateDeltaTime();

        glEnable(GL_DEPTH_TEST);
        glEnable( GL_LIGHTING );

        //Camera
        qglviewer::Camera* _camera = camera();

        //ProjectionMatrix
        GLfloat _projectionMatrixF[16];
        _camera->getProjectionMatrix(_projectionMatrixF);

        //ViewMatrix
        QMatrix4x4 _viewMatrix;
        _viewMatrix.setToIdentity();
        QVector3D _cameraPosition = QVector3D(_camera->position().x, _camera->position().y, _camera->position().z);
        QVector3D _cameraUp = QVector3D(_camera->upVector().x, _camera->upVector().y, _camera->upVector().z);
        _viewMatrix.lookAt(_cameraPosition, mFluid->Center(), _cameraUp);
        GLfloat* _viewMatrixF = _viewMatrix.data();

        if (mFluid)
        {
            mFluid->UpdateFluid(mDeltaTime, mMaxWorkGroupX, mMaxWorkGroupY, mMaxWorkGroupZ);
            mFluid->Render(_projectionMatrixF, _viewMatrixF);
        }

        for(const Container* _container : mContainers)
            _container->Render(_projectionMatrixF, _viewMatrixF);

        update();
    }

    void pickBackgroundColor() {
        QColor _bc = QColorDialog::getColor( this->backgroundColor(), this);
        if( _bc.isValid() ) {
            this->setBackgroundColor( _bc );
            this->update();
        }
    }

    void adjustCamera( QVector3D const & bb , QVector3D const & BB ) {
        QVector3D const & center = ( bb + BB )/2.f;
        setSceneCenter( qglviewer::Vec( 1,1,1 ) );
        setSceneRadius( 1.5f * ( BB - bb ).length() );
        showEntireScene();
    }


    void init() {
        initializeOpenGLFunctions();

        QOpenGLVertexArrayObject vao;
        vao.create();
        vao.bind();

        setMouseTracking(true);// Needed for MouseGrabber.

        setBackgroundColor(QColor(255,255,255));

        // Lights:
        GLTools::initLights();
        GLTools::setSunsetLight();
        GLTools::setDefaultMaterial();

        //
        glShadeModel(GL_SMOOTH);
        glFrontFace(GL_CW); // CCW ou CW

        glEnable(GL_DEPTH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);

        glEnable(GL_CLIP_PLANE0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_COLOR_MATERIAL);

        //
        setSceneCenter( qglviewer::Vec( 0 , 0 , 0 ) );
        setSceneRadius( 100.f );
        showEntireScene();

        mFluid = new Fluid();

        for(uint i = 0; i < mContainers.size(); ++i)
        {
            Container* _container =new Container();
            _container->GetTransform().SetWorldRotation(QVector3D(0,0,0));
            _container->GetCollider()->RefreshColliderTransform();
            mContainers[i] = _container;
        }


        //Get max workGroups
        QOpenGLExtraFunctions _functions = QOpenGLExtraFunctions(QOpenGLContext::currentContext());
        _functions.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &mMaxWorkGroupX);
        _functions.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &mMaxWorkGroupY);
        _functions.glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &mMaxWorkGroupZ);
    }

    QString helpString() const {
        QString text("<h2>Our cool project</h2>");
        text += "<p>";
        text += "This is a research application, it can explode.";
        text += "<h3>Participants</h3>";
        text += "<ul>";
        text += "<li>...</li>";
        text += "</ul>";
        text += "<h3>Basics</h3>";
        text += "<p>";
        text += "<ul>";
        text += "<li>H   :   make this help appear</li>";
        text += "<li>Ctrl + mouse right button double click   :   choose background color</li>";
        text += "<li>Ctrl + T   :   change window title</li>";
        text += "</ul>";
        return text;
    }

    void updateTitle( QString text ) {
        this->setWindowTitle( text );
        emit windowTitleUpdated(text);
    }

    void keyPressEvent( QKeyEvent * event ) {
        if( event->key() == Qt::Key_H ) {
            help();
        }
        else if( event->key() == Qt::Key_T ) {
            if( event->modifiers() & Qt::CTRL )
            {
                bool ok;
                QString text = QInputDialog::getText(this, tr(""), tr("title:"), QLineEdit::Normal,this->windowTitle(), &ok);
                if (ok && !text.isEmpty())
                {
                    updateTitle(text);
                }
            }
        }
    }

    void mouseDoubleClickEvent( QMouseEvent * e )
    {
        if( (e->modifiers() & Qt::ControlModifier)  &&  (e->button() == Qt::RightButton) )
        {
            pickBackgroundColor();
            return;
        }

        if( (e->modifiers() & Qt::ControlModifier)  &&  (e->button() == Qt::LeftButton) )
        {
            showControls();
            return;
        }

        QGLViewer::mouseDoubleClickEvent( e );
    }

    void mousePressEvent(QMouseEvent* e ) {
        QGLViewer::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e  ){
        QGLViewer::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent* e  ) {
        QGLViewer::mouseReleaseEvent(e);
    }

signals:
    void windowTitleUpdated( const QString & );

public slots:
    void open_mesh() {
//        bool success = false;
//        QString fileName = QFileDialog::getOpenFileName(NULL,"","");
//        if ( !fileName.isNull() ) { // got a file name
//            if(fileName.endsWith(QString(".off")))
//                success = OFFIO::openTriMesh(fileName.toStdString() , mesh->mPositions , mesh->mTriangles );
//            else if(fileName.endsWith(QString(".obj")))
//                success = OBJIO::openTriMesh(fileName.toStdString() , mesh->mPositions , mesh->mTriangles );
//            if(success) {
//                std::cout << fileName.toStdString() << " was opened successfully" << std::endl;
//                point3d bb(FLT_MAX,FLT_MAX,FLT_MAX) , BB(-FLT_MAX,-FLT_MAX,-FLT_MAX);
//                for( unsigned int v = 0 ; v < mesh->mPositions.size() ; ++v ) {
//                    bb = point3d::min(bb , mesh->mPositions[v]);
//                    BB = point3d::max(BB , mesh->mPositions[v]);
//                }
//                adjustCamera(bb,BB);
//                update();
//            }
//            else
//                std::cout << fileName.toStdString() << " could not be opened" << std::endl;
//        }
    }

    void save_mesh() {
//        bool success = false;
//        QString fileName = QFileDialog::getOpenFileName(NULL,"","");
//        if ( !fileName.isNull() ) { // got a file name
//            if(fileName.endsWith(QString(".off")))
//                success = OFFIO::save(fileName.toStdString() , mesh->mPositions , mesh->mTriangles );
//            else if(fileName.endsWith(QString(".obj")))
//                success = OBJIO::save(fileName.toStdString() , mesh->mPositions , mesh->mTriangles );
//            if(success)
//                std::cout << fileName.toStdString() << " was saved" << std::endl;
//            else
//                std::cout << fileName.toStdString() << " could not be saved" << std::endl;
//        }
    }

    void showControls()
    {
        // Show controls :
        controls->close();
        controls->show();
    }

    void saveCameraInFile(const QString &filename){
        std::ofstream out (filename.toUtf8());
        if (!out)
            exit (EXIT_FAILURE);
        // << operator for point3 causes linking problem on windows
        out << camera()->position()[0] << " \t" << camera()->position()[1] << " \t" << camera()->position()[2] << " \t" " " <<
                                          camera()->viewDirection()[0] << " \t" << camera()->viewDirection()[1] << " \t" << camera()->viewDirection()[2] << " \t" << " " <<
                                          camera()->upVector()[0] << " \t" << camera()->upVector()[1] << " \t" <<camera()->upVector()[2] << " \t" <<" " <<
                                          camera()->fieldOfView();
        out << std::endl;

        out.close ();
    }

    void openCameraFromFile(const QString &filename){

        std::ifstream file;
        file.open(filename.toStdString().c_str());

        qglviewer::Vec pos;
        qglviewer::Vec view;
        qglviewer::Vec up;
        float fov;

        file >> (pos[0]) >> (pos[1]) >> (pos[2]) >>
                                                    (view[0]) >> (view[1]) >> (view[2]) >>
                                                                                           (up[0]) >> (up[1]) >> (up[2]) >>
                                                                                                                            fov;

        camera()->setPosition(pos);
        camera()->setViewDirection(view);
        camera()->setUpVector(up);
        camera()->setFieldOfView(fov);

        camera()->computeModelViewMatrix();
        camera()->computeProjectionMatrix();

        update();
    }


    void openCamera(){
        QString fileName = QFileDialog::getOpenFileName(NULL,"","*.cam");
        if ( !fileName.isNull() ) {                 // got a file name
            openCameraFromFile(fileName);
        }
    }
    void saveCamera(){
        QString fileName = QFileDialog::getSaveFileName(NULL,"","*.cam");
        if ( !fileName.isNull() ) {                 // got a file name
            saveCameraInFile(fileName);
        }
    }

    void saveSnapShotPlusPlus(){
        QString fileName = QFileDialog::getSaveFileName(NULL,"*.png","");
        if ( !fileName.isNull() ) {                 // got a file name
            setSnapshotFormat("PNG");
            setSnapshotQuality(100);
            saveSnapshot( fileName );
            saveCameraInFile( fileName+QString(".cam") );
        }
    }
};




#endif // MYVIEWER_H
