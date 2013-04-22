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

void Ribbon::draw( const ci::Vec3f &cameraDir )
{
	/*
	gl::begin( GL_LINE_STRIP );
	for ( int i = 0; i < mLoc.size(); i++ )
	{
		gl::vertex( mLoc[ i ] );
	}
	gl::end();
	*/

	gl::begin( GL_QUAD_STRIP );
	int i = 0;
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
	if ( i > 0 )
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
	for ( int i = 0; i < mLoc.size() - 1; i++ )
	{
		dir = mLoc[ i + 1 ] - mLoc[ i ];
		n = dir.cross( cameraDir );
		n.normalize();

		Vec3f n2 = dir.cross( n ).normalized();
		gl::drawLine( mLoc[ i ], mLoc[ i ] + 30 * n2 );
	}
	gl::end();
	*/
}
