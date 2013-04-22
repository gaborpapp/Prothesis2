#include "cinder/gl/gl.h"

#include "Ribbon.h"

using namespace ci;

void Ribbon::update( const Vec3f &pos )
{
	mLoc.push_back( pos );

	if ( mLoc.size() > mMaxLength )
	{
		mLoc.erase( mLoc.begin() );
	}
}

void Ribbon::draw()
{
	gl::begin( GL_LINE_STRIP );
	for ( int i = 0; i < mLoc.size(); i++ )
	{
		gl::vertex( mLoc[ i ] );
	}
	gl::end();
}

