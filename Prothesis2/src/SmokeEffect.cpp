#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/ip/Resize.h"


#include "GlobalData.h"
#include "SmokeEffect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void SmokeEffect::setup()
{
	mParams = mndl::params::PInterfaceGl( GlobalData::get().mControlWindow, "Smoke Effect", Vec2i( 270, 310 ), Vec2i( 232, 16 ) );
	mParams.addPersistentSizeAndPosition();

	// capture
	// list out the capture devices
	vector< ci::Capture::DeviceRef > devices( ci::Capture::getDevices() );

	mDeviceNames.push_back( "Kinect" );
	mCaptures.push_back( ci::CaptureRef() );

	for ( auto deviceIt = devices.cbegin(); deviceIt != devices.cend(); ++deviceIt )
	{
		ci::Capture::DeviceRef device = *deviceIt;
		string deviceName = device->getName(); // + " " + device->getUniqueId();

		try
		{
			if ( device->checkAvailable() )
			{
				mCaptures.push_back( ci::Capture::create( 640, 480, device ) );
				mDeviceNames.push_back( deviceName );
			}
			else
			{
				mCaptures.push_back( ci::CaptureRef() );
				mDeviceNames.push_back( deviceName + " not available" );
			}
		}
		catch ( ci::CaptureExc & )
		{
			ci::app::console() << "Unable to initialize device: " << device->getName() << endl;
		}
	}

	mParams.addText( "Capture source" );
	mParams.addPersistentParam( "Camera", mDeviceNames, &mCurrentCapture, 0 );
	mParams.addSeparator();
	if ( mCurrentCapture >= (int)mCaptures.size() )
		mCurrentCapture = 0;

	mParams.addText ( "Optical flow");
	mParams.addPersistentParam( "Flip", &mFlip, true );
	mParams.addPersistentParam( "Draw flow", &mDrawFlow, false );
	mParams.addPersistentParam( "Draw fluid", &mDrawFluid, true );
	mParams.addPersistentParam( "Draw capture", &mDrawCapture, true );
	mParams.addPersistentParam( "Capture alpha", &mCaptureAlpha, .1f, "min=0 max=1 step=0.05" );
	mParams.addPersistentParam( "Flow multiplier", &mFlowMultiplier, .105, "min=.001 max=2 step=.001" );
	mParams.addPersistentParam( "Flow width", &mOptFlowWidth, 160, "min=20 max=640", true );
	mParams.addPersistentParam( "Flow height", &mOptFlowHeight, 120, "min=20 max=480", true );
	mParams.addSeparator();

	mParams.addText( "Particles" );
	mParams.addPersistentParam( "Particle color", &mParticleColor, Color::white() );
	mParams.addPersistentParam( "Particle aging", &mParticleAging, 0.97f, "min=0 max=1 step=0.001" );
	mParams.addPersistentParam( "Particle min", &mParticleMin, 0, "min=0 max=50" );
	mParams.addPersistentParam( "Particle max", &mParticleMax, 25, "min=0 max=50" );
	mParams.addPersistentParam( "Velocity max", &mMaxVelocity, 7.f, "min=1 max=100" );
	mParams.addPersistentParam( "Velocity particle multiplier", &mVelParticleMult, .57, "min=0 max=2 step=.01" );
	mParams.addPersistentParam( "Velocity particle min", &mVelParticleMin, 1.f, "min=1 max=100 step=.5" );
	mParams.addPersistentParam( "Velocity particle max", &mVelParticleMax, 60.f, "min=1 max=100 step=.5" );
	mParams.addSeparator();

	// fluid
	mParams.addText("Fluid");
	mParams.addPersistentParam( "Fluid width", &mFluidWidth, 160, "min=16 max=512", true );
	mParams.addPersistentParam( "Fluid height", &mFluidHeight, 120, "min=16 max=512", true );
	mParams.addPersistentParam( "Fade speed", &mFluidFadeSpeed, 0.012f, "min=0 max=1 step=0.0005" );
	mParams.addPersistentParam( "Viscosity", &mFluidViscosity, 0.00003f, "min=0 max=1 step=0.00001" );
	mParams.addPersistentParam( "Delta t", &mFluidDeltaT, 0.4f, "min=0 max=10 step=0.05" );
	mParams.addPersistentParam( "Vorticity confinement", &mFluidVorticityConfinement, false );
	mParams.addPersistentParam( "Wrap x", &mFluidWrapX, false );
	mParams.addPersistentParam( "Wrap y", &mFluidWrapY, false );
	mParams.addPersistentParam( "Fluid color", &mFluidColor, Color( 1.f, 0.05f, 0.01f ) );
	mParams.addPersistentParam( "Backgroudn color", &mBackgroundColor, Color::black() );
	mParams.addPersistentParam( "Fluid velocity mult", &mFluidVelocityMult, 10.f, "min=1 max=50 step=0.5" );
	mParams.addPersistentParam( "Fluid color mult", &mFluidColorMult, .5f, "min=0.05 max=10 step=0.05" );

	mFluidSolver.setup( mFluidWidth, mFluidHeight );
	mFluidSolver.enableRGB( false );
	mFluidSolver.setColorDiffusion( 0 );
	mFluidDrawer.enableAlpha( true );
	mFluidDrawer.setup( &mFluidSolver );
	mParams.addButton( "Reset fluid", [&]() { mFluidSolver.reset(); } );

	mParticles.setFluidSolver( &mFluidSolver );
	mParticles.setWindowSize( getSize() );
}

void SmokeEffect::instantiate()
{
	mPrevFrame.release();
}

void SmokeEffect::update()
{
	GlobalData &gd = GlobalData::get();

	static int lastCapture = -1;

	// capture
	// stop and start capture devices
	if ( lastCapture != mCurrentCapture )
	{
		if ( ( lastCapture >= 0 ) && mCaptures[ lastCapture ] )
			mCaptures[ lastCapture ]->stop();

		if ( mCaptures[ mCurrentCapture ] )
			mCaptures[ mCurrentCapture ]->start();

		lastCapture = mCurrentCapture;
	}

	Surface8u captSurf;
	if ( ( mCurrentCapture == 0 ) &&
		gd.mNI && gd.mNI.isCapturing() && gd.mNI.checkNewVideoFrame() )
	{
		captSurf = Surface8u( Channel8u( gd.mNI.getVideoImage() ) );
	}
	else
	if ( mCaptures[ mCurrentCapture ] &&
			mCaptures[ mCurrentCapture ]->isCapturing() &&
			mCaptures[ mCurrentCapture ]->checkNewFrame() )
	{
		captSurf = Surface8u( Channel8u( mCaptures[ mCurrentCapture ]->getSurface() ) );
	}

	if ( captSurf )
	{
		Surface8u smallSurface( mOptFlowWidth, mOptFlowHeight, false );
		if ( ( captSurf.getWidth() != mOptFlowWidth ) ||
				( captSurf.getHeight() != mOptFlowHeight ) )
		{
			ip::resize( captSurf, &smallSurface );
		}
		else
		{
			smallSurface = captSurf;
		}

		mCaptureTexture = gl::Texture( captSurf );

		cv::Mat currentFrame( toOcv( Channel( smallSurface ) ) );
		if ( mFlip )
			cv::flip( currentFrame, currentFrame, 1 );
		if ( ( mPrevFrame.data ) &&
			 ( mPrevFrame.size() == cv::Size( mOptFlowWidth, mOptFlowHeight ) ) )
		{
			double pytScale = .5;
			int levels = 5;
			int winSize = 13;
			int iterations = 5;
			int polyN = 5;
			double polySigma = 1.1;
			int flags = cv::OPTFLOW_FARNEBACK_GAUSSIAN;

			cv::calcOpticalFlowFarneback(
					mPrevFrame, currentFrame,
					mFlow,
					pytScale, levels, winSize, iterations, polyN, polySigma, flags );
		}
		mPrevFrame = currentFrame;

		// fluid update
		if ( mFlow.data )
		{
			RectMapping ofNorm( Area( 0, 0, mFlow.cols, mFlow.rows ),
					Rectf( 0.f, 0.f, 1.f, 1.f ) );
			for ( int y = 0; y < mFlow.rows; y++ )
			{
				for ( int x = 0; x < mFlow.cols; x++ )
				{
					Vec2f v = fromOcv( mFlow.at< cv::Point2f >( y, x ) );
					Vec2f p( x + .5, y + .5 );
					addToFluid( ofNorm.map( p ), ofNorm.map( v ) * mFlowMultiplier );
				}
			}
		}
	}

	// fluid & particles
	mFluidSolver.setFadeSpeed( mFluidFadeSpeed );
	mFluidSolver.setDeltaT( mFluidDeltaT  );
	mFluidSolver.setVisc( mFluidViscosity );
	mFluidSolver.enableVorticityConfinement( mFluidVorticityConfinement );
	mFluidSolver.setWrap( mFluidWrapX, mFluidWrapY );
	mFluidSolver.update();

	mParticles.setAging( mParticleAging );
	mParticles.setColor( mParticleColor );
	mParticles.update( getElapsedSeconds() );
}

void SmokeEffect::addToFluid( const Vec2f &pos, const Vec2f &vel, bool addParticles, bool addForce, bool addColor )
{
	if ( vel.lengthSquared() > 0.000001f )
	{
		Vec2f p;
		p.x = constrain( pos.x, 0.0f, 1.0f );
		p.y = constrain( pos.y, 0.0f, 1.0f );

		if ( addParticles )
		{
			int count = static_cast<int>(
					lmap<float>( vel.length() * mVelParticleMult * getWidth(),
						mVelParticleMin, mVelParticleMax,
						mParticleMin, mParticleMax ) );
			if ( count > 0 )
				mParticles.addParticle( p * Vec2f( getSize() ), count);
		}
		if ( addForce )
			mFluidSolver.addForceAtPos( p, vel * mFluidVelocityMult );

		if ( addColor )
			mFluidSolver.addColorAtPos( p, Color::white() * mFluidColorMult );
	}
}


void SmokeEffect::draw()
{
	gl::setViewport( getBounds() );
	gl::setMatricesWindow( getSize() );
	gl::clear( mBackgroundColor );

	if ( mDrawFluid )
	{
		gl::color( mFluidColor );
		gl::enableAlphaBlending();
		mFluidDrawer.drawColor( 0, 0, getWidth(), getHeight(), true );
		gl::disableAlphaBlending();
	}
	mParticles.draw();

	// draw output to window

	if ( mDrawCapture && mCaptureTexture )
	{
		gl::enableAdditiveBlending();
		gl::color( ColorA( 1, 1, 1, mCaptureAlpha ) );
		mCaptureTexture.enableAndBind();

		gl::pushModelView();
		if ( mFlip )
		{
			gl::translate( getWidth(), 0 );
			gl::scale( -1, 1 );
		}
		gl::drawSolidRect( getBounds() );
		gl::popModelView();
		mCaptureTexture.unbind();
		gl::color( Color::white() );
		gl::disableAlphaBlending();
	}

	// flow vectors, TODO: make this faster using Vbo
	gl::disable( GL_TEXTURE_2D );
	if ( mDrawFlow && mFlow.data )
	{
		RectMapping ofToWin( Area( 0, 0, mFlow.cols, mFlow.rows ),
				getBounds() );
		float ofScale = mFlowMultiplier * getWidth() / (float)mOptFlowWidth;
		gl::color( Color::white() );
		for ( int y = 0; y < mFlow.rows; y++ )
		{
			for ( int x = 0; x < mFlow.cols; x++ )
			{
				Vec2f v = fromOcv( mFlow.at< cv::Point2f >( y, x ) );
				Vec2f p( x + .5, y + .5 );
				gl::drawLine( ofToWin.map( p ),
						ofToWin.map( p + ofScale * v ) );
			}
		}
	}
}

void SmokeEffect::shutdown()
{
	if ( mCaptures[ mCurrentCapture ] )
		mCaptures[ mCurrentCapture ]->stop();
}

