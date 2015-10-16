#include "earth.h"

//
// CONSTRUCTORS ////////////////////////////////////////////////////////////////
// 

Earth::Earth()
    :   Planet( ":/texture/earth.jpg" )
{
    transform.setScale( .75f );
}

//
// OVERRIDDEN FUNCTIONS
// 

void Earth::update()
{
    transform.rotate( 1.0f, 0, 1, 0 );
}