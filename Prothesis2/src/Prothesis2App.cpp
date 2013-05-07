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

	private:
		mndl::params::PInterfaceGl mParams;

		float mFps;
		bool mVerticalSyncEnabled;

		void drawControl();
		void drawOutput();

		vector< EffectRef > mEffects;
		int mEffectIndex;
		int mPrevEffectIndex;

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
	gd.mControlWindow = createWindow( Window::Format().size( 1200, 600 ) );

	mndl::params::PInterfaceGl::load( "params.xml" );

	mParams = mndl::params::PInterfaceGl( gd.mControlWindow, "Parameters", Vec2i( 310, 300 ), Vec2i( 16, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addPersistentParam( "Vertical sync", &mVerticalSyncEnabled, false );
	mParams.addSeparator();

	// output fbo
	gl::Fbo::Format format;
	format.setSamples( 4 );
	mFbo = gl::Fbo( 1024, 768, format );

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
	mKinectThread = thread( bind( &Prothesis2App::openKinect, this, ci::fs::path() ) );
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
	gl::draw( mFbo.getTexture(), getWindowBounds() );
}

void Prothesis2App::drawControl()
{
	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();

	// draw preview
	const int b = 16;
	const int pw = int( getWindowWidth() * .4f );
	const int ph = int( pw / (float)mFbo.getAspectRatio() );

	Rectf previewRect( getWindowWidth() - pw - b, b, getWindowWidth() - b, ph + b );
	gl::color( Color::white() );
	gl::draw( mFbo.getTexture(), previewRect );
	gl::drawString( "Preview", previewRect.getUpperLeft() + Vec2f( b, b ) );
	gl::drawStrokedRect( previewRect );

	mParams.draw();
	for ( auto it = mEffects.cbegin(); it != mEffects.cend(); ++it )
	{
		(*it)->drawControl();
	}
}

void Prothesis2App::mouseDown( ci::app::MouseEvent event )
{
	mEffects[ mEffectIndex ]->mouseDown( event );
}

void Prothesis2App::mouseDrag( ci::app::MouseEvent event )
{
	mEffects[ mEffectIndex ]->mouseDrag( event );
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

