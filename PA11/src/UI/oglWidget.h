#ifndef OGL_WIDGET_H
#define OGL_WIDGET_H

#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <QTime>

#include <QKeyEvent>
#include <QMouseEvent>

#include <QMap>
#include <QMatrix4x4>

#include <QDebug>
#include <QString>
#include <QMediaPlayer>
#include <QPainter>

#include <btBulletDynamicsCommon.h>

#include "Controls/input.h"
#include "Controls/camera3d.h"
#include "Controls/time.h"
#include "3D/renderable.h"
#include "Maze/labyrinth.h"
#include "Maze/ball.h"
#include "Maze/wall.h"

class OGLWidget    :    public QOpenGLWidget,
                        protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OGLWidget();
    OGLWidget( QString team1, QString team2 );
    ~OGLWidget();

    virtual void initializeGL();
    virtual void resizeGL( int width, int height );
    virtual void paintGL();
    virtual void teardownGL();

signals:
    void win();

protected slots:
    void pause();
    void update();

protected:
    void keyPressEvent( QKeyEvent* event );
    void keyReleaseEvent( QKeyEvent* event );
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );

private:
    void initializeBullet();
    void teardownBullet();
    void flyThroughCamera();
    void controlBoard();
    void printContextInfo();
    void checkIfWon();

    // OpenGL Objects
    QMap<QString, Renderable*> renderables;

    // Invisible Bullet object
    Wall* m_invisibleWall;
    int score;

    // 3D data
    QMatrix4x4 projection;
    Camera3D camera;

    // Bullet data
    btBroadphaseInterface* m_broadphase;
    btDefaultCollisionConfiguration* m_collisionConfig;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    // World Timer
    Time updateTimer;

    // Pause flag
    bool isPaused = false;
};

#endif  //  OGL_WIDGET_H