#include "cinder/CinderMath.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "GlobalData.h"
#include "Fade.h"
#include "Resources.h"

using namespace ci;

Fade::Fade( int w, int h )
{
	GlobalData &gd = GlobalData::get();

	gd.mPostProcessingParams->addSeparator();
	gd.mPostProcessingParams->addText( "Fade" );

	gd.mPostProcessingParams->addPersistentParam( "Fade enable", &mEnabled, true );
	mFade = 0.f;
	gd.mPostProcessingParams->addParam( "Fade value", mFade.ptr(), "min=0 max=1 step=0.01 group=Fade" );
	gd.mPostProcessingParams->addPersistentParam( "Fade duration", &mFadeDuration, 2.f, "min=.5 step=.25 group=Fade" );
	gd.mPostProcessingParams->addPersistentParam( "Fade color", &mFadeColor, Color::black(), "group=Fade" );
	gd.mPostProcessingParams->addButton( "Fade color white", [&]() { mFadeColor = Color::white(); }, "group=Fade" );
	gd.mPostProcessingParams->addButton( "Fade color black", [&]() { mFadeColor = Color::black(); }, "group=Fade" );
	gd.mPostProcessingParams->addButton( "Fade out", [&]()
			{
			mFade = 0.f;
			app::timeline().apply( &mFade, 1.f, mFadeDuration );
			}, "group=Fade" );
	gd.mPostProcessingParams->addButton( "Fade in", [&]()
			{
			mFade = 1.f;
			app::timeline().apply( &mFade, 0.f, mFadeDuration );
			}, "group=Fade" );
	gd.mPostProcessingParams->setOptions( "", "refresh=0.05" );

	gl::Fbo::Format fboFormat;
	fboFormat.enableDepthBuffer( false );
	mFbo = gl::Fbo( w, h, fboFormat );

	try
	{
		mShader = gl::GlslProg::create( app::loadResource( RES_FADE_VERT ),
										app::loadResource( RES_FADE_FRAG ) );
	}
	catch ( const gl::GlslProgCompileExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}
	catch ( const app::AssetLoadExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}

}

gl::Texture Fade::process( const ci::gl::Texture &source )
{
	if ( !mEnabled )
		return source;

	gl::SaveFramebufferBinding fboSaver;
	gl::SaveColorState colorSaver;
	gl::pushMatrices();

	mFbo.bindFramebuffer();
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize(), false );

	if ( mShader )
	{
		mShader->bind();
		mShader->uniform( "txt", 0 );
		mShader->uniform( "fadeColor", mFadeColor );
		mShader->uniform( "fadeValue", mFade );
	}
	gl::color( Color::white() );
	gl::draw( source, mFbo.getBounds() );
	if ( mShader )
	{
		mShader->unbind();
	}

	gl::popMatrices();
	return mFbo.getTexture();
}

