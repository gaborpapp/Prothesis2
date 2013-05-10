#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "Feedback.h"
#include "GlobalData.h"
#include "Resources.h"

using namespace ci;

Feedback::Feedback( int w, int h )
{
	GlobalData &gd = GlobalData::get();

	gd.mPostProcessingParams.addSeparator();
	gd.mPostProcessingParams.addText( "Feedback" );
	gd.mPostProcessingParams.addPersistentParam( "Feedback enable", &mEnabled, false );
	gd.mPostProcessingParams.addPersistentParam( "Speed", &mSpeed, 30.f, "min=0 step=.1" );
	gd.mPostProcessingParams.addPersistentParam( "Feed value", &mFeed, 0.95f, "min=0 max=1 step=.005" );

	gd.mPostProcessingParams.addPersistentParam( "Amplitude X", &mExpParams[ 0 ].amplitude, -2.47f,
			"step=.01 group='Displace X'" );
	gd.mPostProcessingParams.addPersistentParam( "Angle start X", &mExpParams[ 0 ].start, 1.72259f,
			"step=.00005 group='Displace X'" );
	gd.mPostProcessingParams.addPersistentParam( "Angle offset X", &mExpParams[ 0 ].offset, -2.08993f,
			"step=.00005 group='Displace X'" );
	gd.mPostProcessingParams.addPersistentParam( "Increment per frame X", &mExpParams[ 0 ].addPerFrame, 0.00778f,
			"step=.00005 group='Displace X'" );
	gd.mPostProcessingParams.addPersistentParam( "Increment per pixel X", &mExpParams[ 0 ].addPerPixel, 0.01896f,
			"step=.00005 group='Displace X'" );
	gd.mPostProcessingParams.setOptions( "Displace X", "opened=false" );

	gd.mPostProcessingParams.addPersistentParam( "Amplitude Y", &mExpParams[ 1 ].amplitude, 3.64,
			"step=.01 group='Displace Y'" );
	gd.mPostProcessingParams.addPersistentParam( "Angle start Y", &mExpParams[ 1 ].start, 1.31368f,
			"step=.00005 group='Displace Y'" );
	gd.mPostProcessingParams.addPersistentParam( "Angle offset Y", &mExpParams[ 1 ].offset, -1.95066f,
			"step=.00005 group='Displace Y'" );
	gd.mPostProcessingParams.addPersistentParam( "Increment per frame Y", &mExpParams[ 1 ].addPerFrame, -0.01268f,
			"step=.00005 group='Displace Y'" );
	gd.mPostProcessingParams.addPersistentParam( "Increment per pixel Y", &mExpParams[ 1 ].addPerPixel, 0.00997f,
			"step=.00005 group='Displace Y'" );
	gd.mPostProcessingParams.setOptions( "Displace Y", "opened=false" );

	gd.mPostProcessingParams.addPersistentParam( "Noise speed", &mNoiseSpeed, 3.99f, "step=.05" );
	gd.mPostProcessingParams.addPersistentParam( "Noise scale", &mNoiseScale, 0.87f, "step=.05" );
	gd.mPostProcessingParams.addPersistentParam( "Noise disp", &mNoiseDisp, .005f, "step=.001" );
	gd.mPostProcessingParams.addPersistentParam( "Noise twirl", &mNoiseTwirl, 4.9f, "step=.1" );

	gd.mPostProcessingParams.addButton( "Randomize feedback", std::bind( &Feedback::randomizeParams, this, 0 ) );

	try
	{
		mShader = gl::GlslProg( app::loadResource( RES_FEEDBACK_VERT ),
								app::loadResource( RES_FEEDBACK_FRAG ) );
	}
	catch ( gl::GlslProgCompileExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}

	gl::Fbo::Format fboFormat;
	fboFormat.enableColorBuffer( true, 2 );
	fboFormat.enableDepthBuffer( false );
	fboFormat.setSamples( 4 );
	mFbo = gl::Fbo( w, h, fboFormat );

	mFbo.bindFramebuffer();
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
	gl::clear();
	glDrawBuffer( GL_COLOR_ATTACHMENT1_EXT );
	gl::clear();
	mFbo.unbindFramebuffer();
}

void Feedback::randomizeParams( unsigned long seed )
{
	if ( seed != 0 )
		Rand::randSeed( seed );

	for ( int i = 0 ; i < sizeof( mExpParams ) / sizeof( mExpParams[ 0 ] ); i++ )
	{
		mExpParams[ i ].amplitude = Rand::randFloat( -5.f, 5.f );
		mExpParams[ i ].start = Rand::randFloat( -M_PI, M_PI );
		mExpParams[ i ].offset = Rand::randFloat( -M_PI, M_PI );
		mExpParams[ i ].addPerFrame = Rand::randFloat( -.1f, .1f );
		mExpParams[ i ].addPerPixel = Rand::randFloat( -.05f, .05f );
	}
	mNoiseSpeed = Rand::randFloat( 0.f, 10.f );
	mNoiseScale = Rand::randFloat( 0.f, 2.f );
	mNoiseDisp = Rand::randFloat( 0.f, 0.01f );
	mNoiseTwirl = Rand::randFloat( 0.f, 6.f );
}

gl::Texture Feedback::process( const ci::gl::Texture &source )
{
	if ( !mEnabled )
		return source;

	static int pingPongId = 0;

	pingPongId ^= 1;

	gl::pushMatrices();

	mFbo.bindFramebuffer();
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + pingPongId );
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize(), false );

	if ( mShader )
	{
		mShader.bind();
		mShader.uniform( "txt", 0 );
		mShader.uniform( "ptxt", 1 );
		mShader.uniform( "feed", mFeed );
		mShader.uniform( "time", (float)( app::getElapsedSeconds() / 60. ) );
		mShader.uniform( "noiseSpeed", mNoiseSpeed );
		mShader.uniform( "noiseScale", mNoiseScale );
		mShader.uniform( "noiseDisp", mNoiseDisp );
		mShader.uniform( "noiseTwirl", mNoiseTwirl );

		float t = (float)app::getElapsedSeconds() * mSpeed;
		mShader.uniform( "dispXOffset", mExpParams[ 0 ].start +
				t * mExpParams[ 0 ].addPerFrame + mExpParams[ 0 ].offset );
		mShader.uniform( "dispXAddPerPixel", mExpParams[ 0 ].addPerPixel * DISP_SIZE );
		mShader.uniform( "dispXAmplitude", mExpParams[ 0 ].amplitude / DISP_SIZE );
		mShader.uniform( "dispYOffset", mExpParams[ 1 ].start +
				t * mExpParams[ 1 ].addPerFrame + mExpParams[ 1 ].offset );
		mShader.uniform( "dispYAddPerPixel", mExpParams[ 1 ].addPerPixel * DISP_SIZE );
		mShader.uniform( "dispYAmplitude", mExpParams[ 1 ].amplitude / DISP_SIZE );
	}

	source.bind();
	mFbo.bindTexture( 1, pingPongId ^ 1 );
	gl::drawSolidRect( mFbo.getBounds() );
	source.unbind();
	mFbo.unbindTexture();

	if ( mShader )
		mShader.unbind();

	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
	mFbo.unbindFramebuffer();

	gl::popMatrices();
	return mFbo.getTexture( pingPongId );
}

