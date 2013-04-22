#include "RibbonManager.h"

using namespace std;
using namespace ci;
using namespace ci::app;

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
		mRibbons[ idRibbon ] = Ribbon::create();

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
