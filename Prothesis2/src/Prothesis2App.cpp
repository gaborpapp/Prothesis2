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

#include "Black.h"

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

		void update();
		void draw();

	private:
		mndl::params::PInterfaceGl mParams;

		float mFps;
		bool mVerticalSyncEnabled = false;

		void drawControl();
		void drawOutput();

		vector< EffectRef > mEffects;
		int mEffectIndex;
		int mPrevEffectIndex;

		gl::Fbo mFbo;
};

void Prothesis2App::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
}

void Prothesis2App::setup()
{
	GlobalData &gd = GlobalData::get();
	gd.mOutputWindow = getWindow();
	gd.mControlWindow = createWindow( Window::Format().size( 800, 600 ) );

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

	vector< string > effectNames;
	for ( auto it = mEffects.cbegin(); it != mEffects.cend(); ++it )
	{
		effectNames.push_back( (*it)->getName() );
		(*it)->resize( mFbo.getSize() );
	}
	mEffectIndex = mPrevEffectIndex = 0;
	mParams.addParam( "Effect", effectNames, &mEffectIndex );
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
}

CINDER_APP_BASIC( Prothesis2App, RendererGl )
