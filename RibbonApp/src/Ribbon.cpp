#include "cinder/gl/gl.h"

#include "Ribbon.h"

using namespace ci;

mndl::params::PInterfaceGl Ribbon::mParams      = mndl::params::PInterfaceGl();
int                        Ribbon::mMaxLength   = 32;
float                      Ribbon::mWidth       = 16.0f;
float                      Ribbon::mMinDistance = 0.5f;

void Ribbon::setup()
{
	mParams = mndl::params::PInterfaceGl( "Ribbon", Vec2i( 200, 150 ), Vec2i( 500, 16 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addPersistentParam( "Max length"  , &mMaxLength  , 32   , "min= 10    max= 1000  step= 1"    );
	mParams.addPersistentParam( "Width"       , &mWidth      , 16.0f, "min= 0.1f  max= 100.0 step= 0.1"  );
	mParams.addPersistentParam( "Min distance", &mMinDistance, 0.5f , "min= 0.01f max= 10.0  step= 0.01" );
}

void Ribbon::update( const Vec3f &pos )
{
	if( ! mActive )
		return;

	if ( !mLoc.empty() && ( mLoc.back().distanceSquared( pos ) < mMinDistance ) )
		return;

	mLoc.push_back( pos );

	while ( mLoc.size() > mMaxLength )
	{
		mLoc.erase( mLoc.begin() );
	}
}

void Ribbon::draw( const ci::Vec3f &cameraDir )
{
	if( ! mActive )
		return;

	if( mLoc.empty() )
		return;

	/*
	gl::begin( GL_LINE_STRIP );
	for ( int i = 0; i < mLoc.size(); i++ )
	{
		gl::vertex( mLoc[ i ] );
	}
	gl::end();
	*/

	gl::begin( GL_QUAD_STRIP );
	size_t i = 0;
	Vec3f dir, off, n;
	Vec3f normal;
	for ( ; i < mLoc.size() - 1; i++ )
	{
		dir = mLoc[ i + 1 ] - mLoc[ i ];
		n = dir.cross( cameraDir );
		n.normalize();

		normal = dir.cross( n ).normalized();

		off = n * mWidth;
		glNormal3fv( &normal.x );
		gl::vertex( mLoc[ i ] - off );
		glNormal3fv( &normal.x );
		gl::vertex( mLoc[ i ] + off );
	}
	// last point
	if ( ( i > 0 ) && ( i < mLoc.size() ) )
	{
		glNormal3fv( &normal.x );
		gl::vertex( mLoc[ i ] - off );
		glNormal3fv( &normal.x );
		gl::vertex( mLoc[ i ] + off );
	}

	gl::end();

	/*
	gl::disable( GL_LIGHTING );
	gl::color( Color( 1, 0, 0 ) );
	for ( unsigned short i = 0; i < mLoc.size() - 1; i++ )
	{
		dir = mLoc[ i + 1 ] - mLoc[ i ];
		n = dir.cross( cameraDir );
		n.normalize();

		Vec3f n2 = dir.cross( n ).normalized();
		gl::drawLine( mLoc[ i ], mLoc[ i ] + 30 * n2 );
	}
	gl::end();
	*/

	mParams.draw();
}

void Ribbon::clear()
{
	mLoc.clear();
}

void Ribbon::setActive( bool active )
{
	if( mActive != active )
	{
		mActive = active;

		if( ! mActive )
			clear();
	}
}

bool Ribbon::getActive() const
{
	return mActive;
}