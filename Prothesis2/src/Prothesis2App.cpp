/*
 Copyright (C) 2013 Gabor Papp, Gabor Botond Barna

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

#include "mndlkit/params/PParams.h"

#include "Effect.h"
#include "GlobalData.h"

#include "BlackEffect.h"
#include "SkelMeshEffect.h"
#include "SmokeEffect.h"
#include "RibbonEffect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Prothesis2App : public AppBasic
{
	public:
		void prepareSettings( Settings *settings );
		void setup();
		void shutdown();

		void keyDown( KeyEvent event );
		void mouseDown( ci::app::MouseEvent event );
		void mouseDrag( ci::app::MouseEvent event );

		void update();
		void draw();

		void resize();

	private:
		mndl::params::PInterfaceGl mParams;

		float mFps;
		bool mVerticalSyncEnabled;

		void drawControl();
		void drawOutput();

		Rectf mPreviewRect;

		vector< EffectRef > mEffects;
		int mEffectIndex;
		int mPrevEffectIndex;

#define FBO_WIDTH 1024
#define FBO_HEIGHT 768
		gl::Fbo mFbo;

		// openni
		std::thread mKinectThread;
		std::string mKinectProgress;
		bool mKinectMirrored;
		float mKinectSmoothing;
		void openKinect( const ci::fs::path &path = ci::fs::path() );
};

void Prothesis2App::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
}

void Prothesis2App::setup()
{
	GlobalData &gd = GlobalData::get();
	gd.mOutputWindow = getWindow();
	gd.mControlWindow = createWindow( Window::Format().size( 1250, 700 ) );

	mndl::params::PInterfaceGl::load( "params.xml" );

	mParams = mndl::params::PInterfaceGl( gd.mControlWindow, "Parameters", Vec2i( 200, 310 ), Vec2i( 16, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addPersistentParam( "Vertical sync", &mVerticalSyncEnabled, false );
	mParams.addSeparator();

	// output fbo
	gl::Fbo::Format format;
	format.setSamples( 4 );
	mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );

	// setup effects
	mEffects.push_back( BlackEffect::create() );
	mEffects.push_back( SmokeEffect::create() );
	mEffects.push_back( SkelMeshEffect::create() );
	mEffects.push_back( RibbonEffect::create() );

	vector< string > effectNames;
	for ( auto it = mEffects.cbegin(); it != mEffects.cend(); ++it )
	{
		effectNames.push_back( (*it)->getName() );
		(*it)->resize( mFbo.getSize() );
		(*it)->setup();
	}
	mEffectIndex = mPrevEffectIndex = 0;
	mParams.addParam( "Effect", effectNames, &mEffectIndex );
	mParams.addSeparator();

	// OpenNI
	mKinectProgress = "Connecting...";
	mParams.addText( "Kinect" );
	mParams.addParam( "Kinect progress", &mKinectProgress, "", true );
//#define USE_KINECT_RECORDING
#ifdef USE_KINECT_RECORDING
	fs::path path = getAppPath();
#ifdef CINDER_MAC
	path /= "/../";
#endif
	path /= "test.oni";
	mKinectThread = thread( bind( &Prothesis2App::openKinect, this, path ) );
#else
	mKinectThread = thread( bind( &Prothesis2App::openKinect, this, fs::path() ) );
#endif
	mParams.addPersistentParam( "Kinect mirror", &mKinectMirrored, false );
	mParams.addPersistentParam( "Kinect smoothing", &mKinectSmoothing, 0.7f, "min=0 max=.99 step=.1" );
	mParams.addSeparator();

	mndl::params::PInterfaceGl::showAllParams( true, true );
}

void Prothesis2App::update()
{
	mFps = getAverageFps();
	if ( mVerticalSyncEnabled != gl::isVerticalSyncEnabled() )
		gl::enableVerticalSync( mVerticalSyncEnabled );

	// update current effect
	if ( mEffectIndex != mPrevEffectIndex )
	{
		mEffects[ mPrevEffectIndex ]->deinstantiate();
		mEffects[ mEffectIndex ]->instantiate();
	}
	mEffects[ mEffectIndex ]->update();
	mPrevEffectIndex = mEffectIndex;

	GlobalData &gd = GlobalData::get();
	if ( gd.mNI )
	{
		if ( gd.mNI.isMirrored() != mKinectMirrored )
			gd.mNI.setMirrored( mKinectMirrored );
		gd.mNIUserTracker.setSmoothing( mKinectSmoothing );
	}
}

void Prothesis2App::draw()
{
	GlobalData &gd = GlobalData::get();
	app::WindowRef currentWindow = getWindow();

	if ( currentWindow == gd.mOutputWindow )
	{
		drawOutput();
	}
	else
	if ( currentWindow == gd.mControlWindow )
	{
		drawControl();
	}
}

void Prothesis2App::drawOutput()
{
	mFbo.bindFramebuffer();
	gl::clear();
	mEffects[ mEffectIndex ]->draw();
	mFbo.unbindFramebuffer();

	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();
	gl::color( Color::white() );
	// flip fbo texture
	gl::pushModelView();
	gl::translate( 0.f, getWindowHeight() );
	gl::scale( 1.f, -1.f );
	gl::draw( mFbo.getTexture(), getWindowBounds() );
	gl::popModelView();
}

void Prothesis2App::drawControl()
{
	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();

	// draw preview
	gl::color( Color::white() );
	// flip fbo texture
	gl::pushModelView();
	gl::translate( 0.f, mPreviewRect.getHeight() + 2 * 16 );
	gl::scale( 1.f, -1.f );
	gl::draw( mFbo.getTexture(), mPreviewRect );
	gl::popModelView();
	gl::drawString( "Preview", mPreviewRect.getUpperLeft() + Vec2f( 16, 16 ) );
	gl::drawStrokedRect( mPreviewRect );

	mParams.draw();
	for ( auto it = mEffects.cbegin(); it != mEffects.cend(); ++it )
	{
		(*it)->drawControl();
	}
}

void Prothesis2App::resize()
{
	// change preview rect
	const int b = 16;
	const int pw = int( getWindowWidth() * .4f );
	const int ph = int( pw / ( FBO_WIDTH / (float)FBO_HEIGHT ) );

	mPreviewRect = Rectf( getWindowWidth() - pw - b, b, getWindowWidth() - b, ph + b );
}

void Prothesis2App::mouseDown( MouseEvent event )
{
	RectMapping mapping;

	if ( event.getWindow() == GlobalData::get().mControlWindow )
	{
		if ( !mPreviewRect.contains( Vec2f( event.getPos() ) ) )
			return;
		// map event from preview to fbo area
		mapping = RectMapping( mPreviewRect, mFbo.getBounds() );
	}
	else
	{
		// map event from output window to fbo area
		mapping = RectMapping( GlobalData::get().mOutputWindow->getBounds(), mFbo.getBounds() );
	}

	Vec2f mappedPos = mapping.map( Vec2f( event.getPos() ) );
	app::MouseEvent ev( event.getWindow(),
			( (unsigned)event.isLeft() * MouseEvent::LEFT_DOWN ) |
			( (unsigned)event.isMiddle() * MouseEvent::MIDDLE_DOWN ) |
			( (unsigned)event.isRight() * MouseEvent::RIGHT_DOWN ),
			(int)mappedPos.x, (int)mappedPos.y,
			( (unsigned)event.isLeftDown() * MouseEvent::LEFT_DOWN ) |
			( (unsigned)event.isMiddleDown() * MouseEvent::MIDDLE_DOWN ) |
			( (unsigned)event.isRightDown() * MouseEvent::RIGHT_DOWN ) |
			( (unsigned)event.isShiftDown() * MouseEvent::SHIFT_DOWN ) |
			( (unsigned)event.isAltDown() * MouseEvent::ALT_DOWN ) |
			( (unsigned)event.isControlDown() * MouseEvent::CTRL_DOWN ) |
			( (unsigned)event.isMetaDown() * MouseEvent::META_DOWN ) |
			( (unsigned)event.isAccelDown() * MouseEvent::ACCEL_DOWN ),
			event.getWheelIncrement(),
			event.getNativeModifiers() );

	mEffects[ mEffectIndex ]->mouseDown( ev );
}

void Prothesis2App::mouseDrag( ci::app::MouseEvent event )
{
	RectMapping mapping;

	if ( event.getWindow() == GlobalData::get().mControlWindow )
	{
		if ( !mPreviewRect.contains( Vec2f( event.getPos() ) ) )
			return;
		// map event from preview to fbo area
		mapping = RectMapping( mPreviewRect, mFbo.getBounds() );
	}
	else
	{
		// map event from output window to fbo area
		mapping = RectMapping( GlobalData::get().mOutputWindow->getBounds(), mFbo.getBounds() );
	}

	Vec2f mappedPos = mapping.map( Vec2f( event.getPos() ) );
	app::MouseEvent ev( event.getWindow(),
			( (unsigned)event.isLeft() * MouseEvent::LEFT_DOWN ) |
			( (unsigned)event.isMiddle() * MouseEvent::MIDDLE_DOWN ) |
			( (unsigned)event.isRight() * MouseEvent::RIGHT_DOWN ),
			(int)mappedPos.x, (int)mappedPos.y,
			( (unsigned)event.isLeftDown() * MouseEvent::LEFT_DOWN ) |
			( (unsigned)event.isMiddleDown() * MouseEvent::MIDDLE_DOWN ) |
			( (unsigned)event.isRightDown() * MouseEvent::RIGHT_DOWN ) |
			( (unsigned)event.isShiftDown() * MouseEvent::SHIFT_DOWN ) |
			( (unsigned)event.isAltDown() * MouseEvent::ALT_DOWN ) |
			( (unsigned)event.isControlDown() * MouseEvent::CTRL_DOWN ) |
			( (unsigned)event.isMetaDown() * MouseEvent::META_DOWN ) |
			( (unsigned)event.isAccelDown() * MouseEvent::ACCEL_DOWN ),
			event.getWheelIncrement(),
			event.getNativeModifiers() );

	mEffects[ mEffectIndex ]->mouseDrag( ev );
}

void Prothesis2App::keyDown( KeyEvent event )
{
	switch ( event.getCode() )
	{
		case KeyEvent::KEY_f:
			setFullScreen( !isFullScreen() );
			break;

		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		default:
			break;
	}
}

void Prothesis2App::shutdown()
{
	mndl::params::PInterfaceGl::save();
	for ( auto it = mEffects.cbegin(); it != mEffects.cend(); ++it )
	{
		(*it)->shutdown();
	}

	mKinectThread.join();
	if ( GlobalData::get().mNI )
		GlobalData::get().mNI.stop();
}

void Prothesis2App::openKinect( const ci::fs::path &path )
{
	GlobalData &gd = GlobalData::get();
	try
	{
		mndl::ni::OpenNI kinect;
		mndl::ni::OpenNI::Options options;
		options.enableDepth( true ).enableImage( true ).enableUserTracker( true );

		if ( path.empty() )
			kinect = mndl::ni::OpenNI( mndl::ni::OpenNI::Device(), options );
		else
			kinect = mndl::ni::OpenNI( path );
		gd.mNI = kinect;
	}
	catch ( const mndl::ni::OpenNIExc &exc )
	{
		if ( path.empty() )
			mKinectProgress = "No device detected";
		else
			mKinectProgress = "Recording not found";
		return;
	}

	if ( path.empty() )
		mKinectProgress = "Connected";
	else
		mKinectProgress = "Recording loaded";

	gd.mNIUserTracker = gd.mNI.getUserTracker();
	gd.mNI.start();
}

CINDER_APP_BASIC( Prothesis2App, RendererGl )

