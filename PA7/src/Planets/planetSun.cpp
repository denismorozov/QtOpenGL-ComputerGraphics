#include "planetSun.h"

//
// CONSTRUCTORS ////////////////////////////////////////////////////////////////
// 

PlanetSun::PlanetSun()
    :   Planet( ":/texture/sun.jpg" )
{
    transform.setScale( 1.0f );
    earth.transform.setScale( .75f );
    mars.transform.setScale( .32f );
}

//
// OVERRIDDEN FUNCTIONS
// 

void PlanetSun::initializeGL()
{
    Planet::initializeGL();
    earth.initializeGL();
    mars.initializeGL();
}

void PlanetSun::paintGL( Camera3D& camera, QMatrix4x4& projection )
{
    Planet::paintGL( camera, projection );
    earth.paintGL( camera, projection );
    mars.paintGL( camera, projection );
}

void PlanetSun::update()
{
    static float translationAngle = 0.0;
    translationAngle += .03;
    earth.transform.setTranslation( 
        transform.translation().x() + 7.0f * sin( translationAngle ),
        transform.translation().y(),
        transform.translation().z() + 2.0f * cos( translationAngle ) );

    transform.rotate( 1.0f, 0, 1, 0 );
    
    earth.update();

    mars.transform.setTranslation( 
        transform.translation().x() + 4.0f * sin( translationAngle ),
        transform.translation().y(),
        transform.translation().z() + 2.0f * cos( translationAngle ) );

    transform.rotate( 1.0f, 0, 1, 0 );
    
    mars.update();
}

void PlanetSun::teardownGL()
{
    Planet::teardownGL();
    earth.teardownGL();
    mars.teardownGL();
}