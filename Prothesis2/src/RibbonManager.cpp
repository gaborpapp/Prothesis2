#include "GlobalData.h"
#include "RibbonManager.h"

using namespace std;
using namespace ci;
using namespace ci::app;

void RibbonManager::update()
{
	for( Ribbons::iterator it = mRibbons.begin(); it != mRibbons.end(); ++it )
	{
		RibbonRef ribbon = it->second;
		ribbon->update();
	}

	// update dying, erase dead ribbons
	for( auto it = mDyingRibbons.begin(); it != mDyingRibbons.end(); )
	{
		if ( (*it)->isAlive() )
		{
			(*it)->update();
			it++;
		}
		else
			it = mDyingRibbons.erase( it );
	}
}

void RibbonManager::draw( const ci::Vec3f& cameraDir )
{
	// active ribbons
	for( Ribbons::const_iterator it = mRibbons.begin(); it != mRibbons.end(); ++it )
	{
		RibbonRef ribbon = it->second;

		ribbon->draw( cameraDir );
	}

	// dying ribbons
	for( auto it = mDyingRibbons.begin(); it != mDyingRibbons.end(); ++it )
	{
		(*it)->draw( cameraDir );
	}
}

void RibbonManager::clear()
{
	for( Ribbons::const_iterator it = mRibbons.begin(); it != mRibbons.end(); ++it )
	{
		RibbonRef ribbon = it->second;
		ribbon->clear();
	}
}

RibbonRef RibbonManager::createRibbon( int id )
{
	if( ! findRibbon( id ))
		mRibbons[ id ] = Ribbon::create();

	return mRibbons[ id ];
}

//! Detaches ribbon from the joint, from now on the ribbon follows the last target set until it dies
void RibbonManager::detachRibbon( int id )
{
	RibbonRef ribbon = findRibbon( id );
	if ( ribbon )
	{
		mDyingRibbons.push_back( ribbon );
		mRibbons.erase( id );
	}
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

