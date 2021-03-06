#include "oglWidget.h"
//
// CONSTRUCTORS ////////////////////////////////////////////////////////////////
// 

/**
 * @brief      Default constructor for OGLWidget.
 */
OGLWidget::OGLWidget()
{
    // Update the widget after a frameswap
    connect( this, SIGNAL( frameSwapped() ),
        this, SLOT( update() ) );

    // Allows keyboard input to fall through
    setFocusPolicy( Qt::ClickFocus );

    isPaused = false;

    m_imgTeam1 = new QPixmap( m_pathToTeam1 );
    m_p1Team = new QLabel( this );
    m_p1Team->setPixmap( *m_imgTeam1 );
    m_p1Team->setScaledContents( true );
    m_imgTeam2 = new QPixmap( m_pathToTeam2 );
    m_p2Team = new QLabel( this );
    m_p2Team->setPixmap( *m_imgTeam2 );
    m_p2Team->setScaledContents( true );

    m_p1Score = 0;
    m_p2Score = 0;

    // Default camera view
    setPerspective( 0 );

    renderables["Table"] = new HockeyTable();
    renderables["Puck"] = new HockeyPuck();
    renderables["Paddle"] = new HockeyPaddle( "Red" );
    renderables["Paddle2"] = new HockeyPaddle( "Blue" );
    renderables["Skybox"] = new Skybox();

    // Note: Actual height of the table is around 30.5
    // So all of these walls are underneath the table but are really tall
    const btVector3 goalSize = btVector3(0.5,35,4.5);
    // red goal
    walls["Goal"] = new Wall(goalSize, btVector3(-30.5, 0, 1));
    // blue goal
    walls["Goal2"] = new Wall(goalSize, btVector3( 33.5, 0, 1));
    // invisible wall in the middle is offset just a little bit to be in table's center
    walls["Middle"] = new Wall(btVector3(0.5,35,20), btVector3(1,0,0));

    // create sound player
	player = new QMediaPlayer(); 

}

/**
 * @brief      Overloaded constructor, sets custom team logos.
 *
 * @param[in]  team1  Name of logo for team 1.
 * @param[in]  team2  Name of logo for team 2.
 */
OGLWidget::OGLWidget( QString team1, QString team2 )
{
    m_pathToTeam1 = "images/Team Logos/" + team1 + ".png";
    m_pathToTeam2 = "images/Team Logos/" + team2 + ".png";

    // Update the widget after a frameswap
    connect( this, SIGNAL( frameSwapped() ),
        this, SLOT( update() ) );

    // Allows keyboard input to fall through
    setFocusPolicy( Qt::ClickFocus );

    isPaused = false;

    m_imgTeam1 = new QPixmap( m_pathToTeam1 );
    m_p1Team = new QLabel( this );
    m_p1Team->setPixmap( *m_imgTeam1 );
    m_p1Team->setScaledContents( true );
    m_imgTeam2 = new QPixmap( m_pathToTeam2 );
    m_p2Team = new QLabel( this );
    m_p2Team->setPixmap( *m_imgTeam2 );
    m_p2Team->setScaledContents( true );

    m_p1Score = 0;
    m_p2Score = 0;

    // Default camera view
    setPerspective( 1 );

    renderables["Table"] = new HockeyTable();
    renderables["Puck"] = new HockeyPuck();
    renderables["Paddle"] = new HockeyPaddle( "Red" );
    renderables["Paddle2"] = new HockeyPaddle( "Blue" );
    renderables["Skybox"] = new Skybox();

    // Note: Actual height of the table is around 30.5
    // So all of these walls are underneath the table but are really tall
    const btVector3 goalSize = btVector3(0.5,35,4.5);
    // red goal
    walls["Goal"] = new Wall(goalSize, btVector3(-30.5, 0, 1));
    // blue goal
    walls["Goal2"] = new Wall(goalSize, btVector3( 33.5, 0, 1));
    // invisible wall in the middle is offset just a little bit to be in table's center
    walls["Middle"] = new Wall(btVector3(0.5,35,20), btVector3(1,0,0));

    // create sound player
    player = new QMediaPlayer(); 
}

/**
 * @brief      Destructor class to unallocate OpenGL information.
 */
OGLWidget::~OGLWidget()
{
    makeCurrent();
    teardownGL();
    teardownBullet();
}

//
// OPENGL FUNCTIONS ////////////////////////////////////////////////////////////
// 

/**
 * @brief      Initializes any OpenGL operations.
 */
void OGLWidget::initializeGL()
{
    // Init OpenGL Backend
    initializeOpenGLFunctions();
    printContextInfo();
    initializeBullet();

    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->initializeGL();
    }

    // renderables
    m_dynamicsWorld->addRigidBody(
        ((HockeyTable*)renderables["Table"])->RigidBody, COL_TABLE, m_TableCollidesWith
    );
    m_dynamicsWorld->addRigidBody(
        ((HockeyPuck*)renderables["Puck"])->RigidBody, COL_PUCK, m_PuckCollidesWith
    );
    m_dynamicsWorld->addRigidBody(
        ((HockeyPaddle*)renderables["Paddle"])->RigidBody, COL_PADDLE, m_PaddleCollidesWith
    );
    m_dynamicsWorld->addRigidBody(
        ((HockeyPaddle*)renderables["Paddle2"])->RigidBody, COL_PADDLE, m_PaddleCollidesWith
    );

    // walls
    m_dynamicsWorld->addRigidBody(
        walls["Goal"]->RigidBody, COL_GOAL, m_GoalCollidesWith
    );
    m_dynamicsWorld->addRigidBody(
        walls["Goal2"]->RigidBody, COL_GOAL, m_GoalCollidesWith
    );
    m_dynamicsWorld->addRigidBody(
        walls["Middle"]->RigidBody, COL_MIDDLE, m_MiddleCollidesWith
    );
}

/**
 * @brief      Sets the prespective whenever the window is resized.
 *
 * @param[in]  width   The width of the new window.
 * @param[in]  height  The height of the new window.
 */
void OGLWidget::resizeGL( int width, int height )
{
    projection.setToIdentity();
    projection.perspective( 55.0f,  // Field of view angle
                            float( width ) / float( height ),   // Aspect Ratio
                            0.001f,  // Near Plane (MUST BE GREATER THAN 0)
                            1500.0f );  // Far Plane

    m_p1Team->setGeometry( QWidget::width() / 2 - m_p1Team->width() / 2 - 120, 
        10, 80, 80 );    
    m_p2Team->setGeometry( QWidget::width() / 2 - m_p2Team->width() / 2 + 120, 
        10, 80, 80 );
}

/**
 * @brief      OpenGL function to draw elements to the surface.
 */
void OGLWidget::paintGL()
{
    // Set the default OpenGL states
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glDepthMask( GL_TRUE );
    glEnable( GL_CULL_FACE );
    glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );

    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->paintGL( camera, projection );
    }

    // 2D Elements
    QFont NHLFont( "NHL", 47 );
    QFont ConsolasFont( "Consolas", 35, QFont::Bold );

    // Draw 2D Elements
    QPainter painter(this);
    QRect rect( 0, 10, QWidget::width(), QWidget::height() / 4 );
    painter.beginNativePainting();

    painter.setPen( QColor( 255, 255, 255, 255 ) );
    painter.setFont( ConsolasFont );
    painter.drawText( rect, Qt::AlignHCenter, QString::number( m_p1Score ) 
        + " - " + QString::number( m_p2Score ) );

    painter.endNativePainting();
}

/**
 * @brief      Destroys any OpenGL data.
 */
void OGLWidget::teardownGL()
{
    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->teardownGL();
    }
}

//
// SLOTS ///////////////////////////////////////////////////////////////////////
// 

/**
 * @brief      Updates any user interactions and model transformations.
 */
void OGLWidget::update()
{
    Input::update();

    flyThroughCamera();

    btVector3 linearVelocity(0,0,0);

    if( Input::keyPressed( Qt::Key_Up ) )
    {
        linearVelocity[0] = -5;

    }
    else if( Input::keyPressed( Qt::Key_Down ) ){
        linearVelocity[0] = 5;
    }

    if( Input::keyPressed( Qt::Key_Left ) ){
        linearVelocity[2] = 5;
    }
    else if( Input::keyPressed( Qt::Key_Right ) )
    {
        linearVelocity[2] = -5;
    }

if( isPaused == false )
{
    ((HockeyPaddle*)renderables["Paddle2"])->RigidBody->setLinearVelocity(
        linearVelocity
    );

    btVector3 linearVelocity2(0,0,0);

    if( Input::keyPressed( Qt::Key_I ) )
    {
        linearVelocity2[0] = -5;

    }
    else if( Input::keyPressed( Qt::Key_K ) ){
        linearVelocity2[0] = 5;
    }

    if( Input::keyPressed( Qt::Key_J ) ){
        linearVelocity2[2] = 5;
    }
    else if( Input::keyPressed( Qt::Key_L ) )
    {
        linearVelocity2[2] = -5;
    }

    ((HockeyPaddle*)renderables["Paddle"])->RigidBody->setLinearVelocity(
        linearVelocity2
    );

    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->update();
    }

    m_dynamicsWorld->stepSimulation( 1, 10 );

    // p1 
    GoalCallback goalCallback(this);
    m_dynamicsWorld->contactTest(
        walls["Goal"]->RigidBody,
        goalCallback
    );

    Goal2Callback goal2Callback(this);
    m_dynamicsWorld->contactTest(
        walls["Goal2"]->RigidBody,
        goal2Callback
    );

    PuckCallback puckCallback(this);
    m_dynamicsWorld->contactTest(
    	((HockeyPuck*)renderables["Puck"])->RigidBody,
    	puckCallback
    );

    /* Example of contactPairTest
    m_dynamicsWorld->contactPairTest( 
        ((HockeyPaddle*)renderables["Paddle2"])->RigidBody,
        ((HockeyPuck*)renderables["Puck"])->RigidBody,
        callback
    );
    */
}
    QOpenGLWidget::update();
}

/**
 * @brief      Public slot to invert the pause state of the game.
 */
void OGLWidget::pause()
{
    isPaused = !isPaused;
}

/**
 * @brief      Sets the camera's perspective to predefined values.
 * details     0: Behind player 1. 1: Behind player 2. 2: Side perspective.
 * 3: Top-Down perspective.
 *
 * @param[in]  perspective  The index of the perspective desired.
 */
void OGLWidget::setPerspective( int perspective )
{
    switch( perspective )
    {
        //Behind player 1
        case 0:
        camera.setRotation( -32.0f, 1.0f, 0.0f, 0.0f );
        camera.rotate( -90.0f, 0.0f, 1.0f, 0.0f );
        camera.setTranslation( -55.8f, 56.7f, 0.0f );
        break;
        //Behind player 2
        case 1:
        camera.setRotation( -32.0f, 1.0f, 0.0f, 0.0f );
        camera.rotate( 90.0f, 0.0f, 1.0f, 0.0f );
        camera.setTranslation( 57.8f, 56.7f, 0.0f );
        break;
        //Side perspective
        case 2:
        camera.setRotation( -45.0f, 1.0f, 0.0f, 0.0f );
        camera.setTranslation( 1.7f, 70.0f, 50.0f );
        break;
        //Top down perspective
        case 3:
        camera.setRotation( -90.0f, 1.0f, 0.0f, 0.0f );
        camera.setTranslation( 1.9f, 85.0f, 0.0f );
        break;
        default:
        break;
    }
}

//
// INPUT EVENTS ////////////////////////////////////////////////////////////////
// 

/**
 * @brief      Default slot for handling key press events.
 *
 * @param      event  The key event information.
 */
void OGLWidget::keyPressEvent( QKeyEvent* event )
{
    if( event->isAutoRepeat() )
        event->ignore();
    else
        Input::registerKeyPress( event->key() );
}

/**
 * @brief      Default slot for handling key release events.
 *
 * @param      event  The key event information.
 */
void OGLWidget::keyReleaseEvent( QKeyEvent* event )
{
    if( event->isAutoRepeat() )
        event->ignore();
    else
        Input::registerKeyRelease( event->key() );
}

/**
 * @brief      Default slot for handling mouse press events.
 *
 * @param      event  The mouse event information.
 */
void OGLWidget::mousePressEvent( QMouseEvent* event )
{
    Input::registerMousePress( event->button() );
}

/**
 * @brief      Default slot for handling mouse release events.
 *
 * @param      event  The mouse event information.
 */
void OGLWidget::mouseReleaseEvent( QMouseEvent* event )
{
    Input::registerMouseRelease( event->button() );
}

//
// PRIVATE HELPER FUNCTIONS ////////////////////////////////////////////////////
//

/**
 * @brief      Helper function to initialize bullet data.
 */
void OGLWidget::initializeBullet()
{
    m_broadphase = new btDbvtBroadphase();
    m_collisionConfig = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher( m_collisionConfig );
    m_solver = new btSequentialImpulseConstraintSolver();
    m_dynamicsWorld = new btDiscreteDynamicsWorld( m_dispatcher, m_broadphase,
        m_solver, m_collisionConfig );

    m_dynamicsWorld->setGravity( btVector3( 0, -9.8, 0 ) );
}

/**
 * @brief      Helper function to delete bullet allocations.
 */
void OGLWidget::teardownBullet()
{
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_dispatcher;
    delete m_collisionConfig;
    delete m_broadphase;
}

/**
 * @brief      Updates the main camera to behave like a Fly-Through Camera.
 */
void OGLWidget::flyThroughCamera()
{
    static const float cameraTranslationSpeed = 0.03f;
    static const float cameraRotationSpeed = 0.2f;

    if( Input::buttonPressed( Qt::RightButton ) )
    {
        // Rotate the camera based on mouse movement
        camera.rotate( -cameraRotationSpeed * Input::mouseDelta().x(), 
            Camera3D::LocalUp );
        camera.rotate( -cameraRotationSpeed * Input::mouseDelta().y(),
            camera.right() );
    }

    // Translate the camera based on keyboard input
    QVector3D cameraTranslations;
    if( Input::keyPressed( Qt::Key_W ) )
        cameraTranslations += camera.forward();
    if( Input::keyPressed( Qt::Key_S ) )
        cameraTranslations -= camera.forward();
    if( Input::keyPressed( Qt::Key_A ) )
        cameraTranslations -= camera.right();
    if( Input::keyPressed( Qt::Key_D ) )
        cameraTranslations += camera.right();
    if( Input::keyPressed( Qt::Key_Q ) )
        cameraTranslations -= camera.up();
    if( Input::keyPressed( Qt::Key_E ) )
        cameraTranslations += camera.up();
    camera.translate( cameraTranslationSpeed * cameraTranslations );

    /* Insanely useful for debugging
    std::cout << camera.translation().x() << ' '
              << camera.translation().y() << ' '
              << camera.translation().z() << std::endl;
    */
    
} 

/**
 * @brief      Helper function to print OpenGL Context information to the debug.
 */
void OGLWidget::printContextInfo()
{
    QString glType;
    QString glVersion;
    QString glProfile;

    // Get Version Information
    glType = ( context()->isOpenGLES() ) ? "OpenGL ES" : "OpenGL";
    glVersion = reinterpret_cast<const char*>( glGetString( GL_VERSION ) );

    // Get Profile Information
    switch( format().profile() )
    {
        case QSurfaceFormat::NoProfile:
            glProfile = "No Profile";
            break;
        case QSurfaceFormat::CoreProfile:
            glProfile = "Core Profile";
            break;
        case QSurfaceFormat::CompatibilityProfile:
            glProfile = "Compatibility Profile";
            break;
    }

    qDebug() << qPrintable( glType ) << qPrintable( glVersion ) << 
        "(" << qPrintable( glProfile ) << ")";
}

/**
 * @brief      Resets the game state after a goal.
 */
void OGLWidget::processGoal()
{
    // play goal sound
    const QString file = "sounds/goal.mp3";
    QUrl url = QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath());
    player->setMedia(url);
    player->setVolume(800);	 
    player->play();

    // reset puck
    btTransform startingState = btTransform( btQuaternion( 0, 0, 0, 1 ), 
        btVector3( 0, 30.5, 0 ) );
    btDefaultMotionState* motionState = new btDefaultMotionState( startingState );
    ((HockeyPuck*)renderables["Puck"])->RigidBody->setMotionState( motionState );
    ((HockeyPuck*)renderables["Puck"])->RigidBody->setLinearVelocity(
        btVector3(0,0,0)
    );
}

/**
 * @brief      Plays a sound whenever a puck collides.
 */
void OGLWidget::puckContactSound()
{
   	const QString file = "sounds/collision.mp3";
    QUrl url = QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath());
    player->setMedia(url);
    player->setVolume(500);	 
    player->play();

}