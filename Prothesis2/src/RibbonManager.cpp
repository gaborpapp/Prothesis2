#include "RibbonManager.h"

using namespace std;
using namespace ci;
using namespace ci::app;

void RibbonManager::setup()
{
	mParams = mndl::params::PInterfaceGl( "RibbonManager", Vec2i( 200, 150 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addPersistentParam( "Max length"  , &mMaxLength  , 32   , "min= 10    max= 1000  step= 1"    );
	mParams.addPersistentParam( "Width"       , &mWidth      , 16.0f, "min= 0.1f  max= 100.0 step= 0.1"  );
	mParams.addPersistentParam( "Min distance", &mMinDistance, 0.5f , "min= 0.01f max= 10.0  step= 0.01" );
}

void RibbonManager::update( int id, Vec3f pos )
{
	RibbonRef ribbon = findRibbon( id );

	if( ribbon )
		ribbon->update( pos );
}

void RibbonManager::draw( const ci::Vec3f& cameraDir )
{
	for( Ribbons::const_iterator it = mRibbons.begin(); it != mRibbons.end(); ++it )
	{
		RibbonRef ribbon = it->second;

		ribbon->draw( cameraDir );
	}
}

void RibbonManager::drawControl()
{
	mParams.draw();
}

void RibbonManager::setActive( int id, bool active )
{
	RibbonRef ribbon = findRibbon( id );

	if( ribbon )
		ribbon->setActive( active );
}

void RibbonManager::clear()
{
	for( Ribbons::const_iterator it = mRibbons.begin(); it != mRibbons.end(); ++it )
	{
		RibbonRef ribbon = it->second;
		ribbon->clear();
	}
}

int RibbonManager::createRibbon( int id )
{
	int idRibbon = id;

	if( ! findRibbon( idRibbon ))
		mRibbons[ idRibbon ] = Ribbon::create( this );

	return idRibbon;
}

void RibbonManager::destroyRibbon( int id )
{
	if( ! findRibbon( id ))
		return;

	mRibbons.erase( id );
}

RibbonRef RibbonManager::findRibbon( int id )
{
	Ribbons::const_iterator it = mRibbons.find( id );
	if( it == mRibbons.end())
		return RibbonRef();

	return it->second;
}

int RibbonManager::getMaxLength() const
{
	return mMaxLength;
}

float RibbonManager::getWidth() const
{
	return mWidth;
}

float RibbonManager::getMinDistance() const
{
	return mMinDistance;
}