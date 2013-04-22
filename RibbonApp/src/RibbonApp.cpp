/*
 Copyright (C) 2013 Gabor Botond Barna, Gabor Papp

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

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Material.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Quaternion.h"

#include "mndlkit/params/PParams.h"

#include "NIUser.h"

#define USE_KINECT_RECORD 1

using namespace ci;
using namespace ci::app;
using namespace std;

class RibbonApp : public AppBasic
{
	public:
		RibbonApp();

		void prepareSettings( Settings *settings );
		void setup();
		void shutdown();

		void keyDown( KeyEvent event );
		void keyUp( KeyEvent event );
		void mouseDown( MouseEvent event );
		void mouseDrag( MouseEvent event );
		void resize();

		void update();
		void draw();

	private:
		mndl::params::PInterfaceGl mParams;

		float mFps;
		bool mVerticalSyncEnabled;

		MayaCamUI    mMayaCam;
		UserManager  mUserManager;

		Vec3f mLightDirection;
		Color mLightAmbient;
		Color mLightDiffuse;
		Color mLightSpecular;

		Color mMaterialAmbient;
		Color mMaterialDiffuse;
		Color mMaterialSpecular;
		float mMaterialShininess;
};

RibbonApp::RibbonApp()
: mVerticalSyncEnabled( false )
{
}

void RibbonApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void RibbonApp::setup()
{
	mndl::params::PInterfaceGl::load( "params.xml" );
	mParams = mndl::params::PInterfaceGl( "Parameters", Vec2i( 200, 300 ), Vec2i( 16, 16 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addParam( "Vertical sync", &mVerticalSyncEnabled );
	mParams.addSeparator();
	mParams.addPersistentParam( "Light direction", &mLightDirection, -Vec3f::zAxis() );
	mParams.addPersistentParam( "Light ambient", &mLightAmbient, Color::black() );
	mParams.addPersistentParam( "Light diffuse", &mLightDiffuse, Color::white() );
	mParams.addPersistentParam( "Light specular", &mLightDiffuse, Color::white() );
	mParams.addSeparator();
	mParams.addPersistentParam( "Material ambient", &mMaterialAmbient, Color::black() );
	mParams.addPersistentParam( "Material diffuse", &mMaterialDiffuse, Color::gray( .5f ) );
	mParams.addPersistentParam( "Material specular", &mMaterialSpecular, Color::white() );
	mParams.addPersistentParam( "Material shininess", &mMaterialShininess, 50.f, "min=0 max=10000 step=.5" );
	mParams.addSeparator();

	CameraPersp cam;
	cam.setPerspective( 60, getWindowAspectRatio(), 1, 15000 );
	cam.setEyePoint( Vec3f( 0, 0, 0 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 800 ) );
	mMayaCam.setCurrentCam( cam );

	gl::enableDepthRead();
	gl::enableDepthWrite();

	try
	{
#if USE_KINECT_RECORD == 0
		mUserManager.setup();
#else
		// use openni recording
		fs::path recordingPath = getAssetPath( "test.oni" );
		mUserManager.setup( recordingPath );
#endif /* USE_KINECT_RECORD */
	}
	catch ( const exception &exc )
	{
		console() << exc.what() << endl;
		quit();
	}
}

void RibbonApp::update()
{
	mFps = getAverageFps();

	if ( mVerticalSyncEnabled != gl::isVerticalSyncEnabled() )
		gl::enableVerticalSync( mVerticalSyncEnabled );

	mUserManager.update();

// 	double t = getElapsedSeconds();
// 	Vec3f pos( math< double >::cos( t * 3.9723231 ),
// 			   math< double >::sin( t * 5.7995332 ),
// 			   math< double >::cos( t * 7.6623417 ) );
// 
// 	mRibbonRef->update( Vec3f( 0, 0, 800 ) + pos * 150.f );
}

void RibbonApp::draw()
{
	gl::clear();
	gl::setViewport( getWindowBounds() );
	gl::setMatrices( mMayaCam.getCamera() );

	gl::color( Color::white() );
//	mRibbonRef->draw();

	Vec3f cameraRight, cameraUp;
	mMayaCam.getCamera().getBillboardVectors( &cameraRight, &cameraUp );
	Vec3f cameraDir = cameraUp.cross( cameraRight );

	// setup light 0
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	//light.setDirection( mLightDirection );
	Quatf q( -Vec3f::zAxis(), mLightDirection );
	light.setDirection( -cameraDir * q );
	light.setAmbient( mLightAmbient );
	light.setDiffuse( mLightDiffuse );
	light.setSpecular( mLightSpecular );
	light.enable();

	gl::Material material( mMaterialAmbient, mMaterialDiffuse, mMaterialSpecular );
	material.setShininess( mMaterialShininess );
	material.apply();

	//gl::enable( GL_LIGHTING );
	mUserManager.draw( cameraDir );
	//gl::disable( GL_LIGHTING );

	mParams.draw();
}

void RibbonApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() )
	{
		case KeyEvent::KEY_f:
			if ( !isFullScreen() )
			{
				setFullScreen( true );
				if ( mParams.isVisible() )
					showCursor();
				else
					hideCursor();
			}
			else
			{
				setFullScreen( false );
				showCursor();
			}
			break;

		case KeyEvent::KEY_s:
			mndl::params::PInterfaceGl::showAllParams( !mParams.isVisible() );
			if ( isFullScreen() )
			{
				if ( mParams.isVisible() )
					showCursor();
				else
					hideCursor();
			}
			break;

		case KeyEvent::KEY_v:
			 mVerticalSyncEnabled = !mVerticalSyncEnabled;
			 break;

		case KeyEvent::KEY_SPACE:
			mUserManager.clearRibbons();
			break;

		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		default:
			break;
	}
}

void RibbonApp::keyUp( KeyEvent event )
{
	mUserManager.keyUp( event );
}

void RibbonApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void RibbonApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void RibbonApp::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void RibbonApp::shutdown()
{
	mndl::params::PInterfaceGl::save();
}

CINDER_APP_BASIC( RibbonApp, RendererGl )

