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
}

/**
 * @brief      Destructor class to unallocate OpenGL information.
 */
OGLWidget::~OGLWidget()
{
    makeCurrent();
    teardownGL();
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

    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->initializeGL();
    }
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

    for( QMap<QString, Renderable*>::iterator iter = renderables.begin(); 
        iter != renderables.end(); iter++ )
    {
        (*iter)->update();
    }

    QOpenGLWidget::update();
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
 * @brief      Updates the main camera to behave like a Fly-Through Camera.
 */
void OGLWidget::flyThroughCamera()
{
    static const float cameraTranslationSpeed = 0.3f;
    static const float cameraRotationSpeed = 0.1f;

    // Rotate the camera based on mouse movement
    camera.rotate( -cameraRotationSpeed * Input::mouseDelta().x(), 
        Camera3D::LocalUp );
    camera.rotate( -cameraRotationSpeed * Input::mouseDelta().y(),
        camera.right() );

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