#include "cinder/gl/gl.h"

#include "Mirror.h"
#include "GlobalData.h"

using namespace ci;

Mirror::Mirror( int w, int h )
{
	GlobalData &gd = GlobalData::get();

	gd.mPostProcessingParams.addSeparator();
	gd.mPostProcessingParams.addText( "Mirror" );
	gd.mPostProcessingParams.addPersistentParam( "Mirror enable", &mEnabled, false );
	gd.mPostProcessingParams.addPersistentParam( "Mirror X", &mFlipHorizontal, false );
	gd.mPostProcessingParams.addPersistentParam( "Mirror Y", &mFlipVertical, false );

	gl::Fbo::Format fboFormat;
	fboFormat.enableDepthBuffer( false );
	fboFormat.setSamples( 4 );
	mFbo = gl::Fbo( w, h, fboFormat );
}

gl::Texture Mirror::process( const ci::gl::Texture &source )
{
	if ( !mEnabled )
		return source;

	gl::pushMatrices();

	mFbo.bindFramebuffer();
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize(), false );

	if ( mFlipVertical )
	{
		gl::translate( Vec2f( 0.f, mFbo.getHeight() ) );
		gl::scale( Vec2f( 1.f, -1.f ) );
	}
	if ( mFlipHorizontal )
	{
		gl::translate( Vec2f( mFbo.getWidth(), 0.f ) );
		gl::scale( Vec2f( -1.f, 1.f ) );
	}
	gl::draw( source, mFbo.getBounds() );

	mFbo.unbindFramebuffer();

	gl::popMatrices();
	return mFbo.getTexture();
}

