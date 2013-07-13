#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "GlobalData.h"
#include "Kaleidoscope.h"
#include "Resources.h"

using namespace ci;

Kaleidoscope::Kaleidoscope( int w, int h )
{
	GlobalData &gd = GlobalData::get();

	gd.mPostProcessingParams.addSeparator();
	gd.mPostProcessingParams.addText( "Kaleidoscope" );
	gd.mPostProcessingParams.addPersistentParam( "Kaleidoscope enable", &mEnabled, false );
	gd.mPostProcessingParams.addPersistentParam( "Reflection lines", &mNumReflectionLines, 3, "min=0 max=32" );
	gd.mPostProcessingParams.addPersistentParam( "Reflection rotation", &mRotation, 0.f, "step=.01" );

	gd.mPostProcessingParams.addPersistentParam( "Kaleidoscope X", &mCenter.x, .5f,
			"min=0 max=1 step=.005 group='Kaleidoscope Center'" );
	gd.mPostProcessingParams.addPersistentParam( "Kaleidoscope Y", &mCenter.y, .5f,
			"min=0 max=1 step=.005 group='Kaleidoscope Center'" );

	gl::Fbo::Format fboFormat;
	fboFormat.enableDepthBuffer( false );
	fboFormat.setSamples( 4 );
	mFbo = gl::Fbo( w, h, fboFormat );

	try
	{
		mShader = gl::GlslProg( app::loadResource( RES_KALEIDOSCOPE_VERT ),
								app::loadResource( RES_KALEIDOSCOPE_FRAG ) );
	}
	catch ( gl::GlslProgCompileExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}
}

gl::Texture Kaleidoscope::process( const ci::gl::Texture &source )
{
	if ( !mEnabled )
		return source;

	gl::SaveFramebufferBinding fboSaver;
	gl::pushMatrices();

	mFbo.bindFramebuffer();
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize(), false );

	if ( mShader )
	{
		mShader.bind();
		mShader.uniform( "center", mCenter );
		mShader.uniform( "numReflectionLines", mNumReflectionLines );
		mShader.uniform( "rotation", mRotation );
		mShader.uniform( "txt", 0 );
	}

	gl::draw( source, mFbo.getBounds() );

	if ( mShader )
		mShader.unbind();

	gl::popMatrices();
	return mFbo.getTexture();
}

