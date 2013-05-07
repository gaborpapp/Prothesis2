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

#include <string>
#include <boost/assign/std/vector.hpp>

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Material.h"
#include "cinder/gl/Light.h"
#include "cinder/TriMesh.h"

#include "Resources.h"

#include "GlobalData.h"
#include "RibbonEffect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void RibbonEffect::setup()
{
	mParams = mndl::params::PInterfaceGl( GlobalData::get().mControlWindow, "Ribbon Effect", Vec2i( 200, 300 ) );
	mParams.addPersistentSizeAndPosition();
	/*
	mParams.addPersistentParam( "Light direction", &mLightDirection, -Vec3f::zAxis() );
	mParams.addPersistentParam( "Light ambient", &mLightAmbient, Color::black() );
	mParams.addPersistentParam( "Light diffuse", &mLightDiffuse, Color::white() );
	mParams.addPersistentParam( "Light specular", &mLightDiffuse, Color::white() );
	mParams.addSeparator();
	mParams.addPersistentParam( "Material ambient", &mMaterialAmbient, Color::black() );
	*/
	mParams.addPersistentParam( "Material diffuse", &mMaterialDiffuse, Color::gray( .5f ) );
	/*
	mParams.addPersistentParam( "Material specular", &mMaterialSpecular, Color::white() );
	mParams.addPersistentParam( "Material shininess", &mMaterialShininess, 50.f, "min=0 max=10000 step=.5" );
	*/
	mParams.addSeparator();
	mParams.addButton( "Reset", [&]() { mRibbonManager.clear(); } );

	CameraPersp cam;
	cam.setPerspective( 60, getAspectRatio(), 1, 15000 );
	cam.setEyePoint( Vec3f( 0, 0, 0 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 800 ) );
	cam.setWorldUp( Vec3f( 0, -1, 0 ) );
	mMayaCam.setCurrentCam( cam );

	mRibbonManager.setup();
	// TODO: add mouse callbacks
}

void RibbonEffect::update()
{
	GlobalData &gd = GlobalData::get();
	if ( !gd.mNI )
		return;

	// this translates the params optionmenu index to the skeleton joint id, the order is important
	const XnSkeletonJoint jointIds[] = {
		XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
		XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
		XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
		XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
		XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };

	vector< unsigned > users = gd.mNIUserTracker.getUsers();
	if ( !users.empty() )
	{
		// one user only
		unsigned userId = users[ 0 ];

		for ( int i = 0; i < sizeof( jointIds ) / sizeof( jointIds[ 0 ] ); i++ )
		{
			XnSkeletonJoint jointId = jointIds[ i ];
			float jointConf;
			Vec3f joint3d = gd.mNIUserTracker.getJoint3d( userId, jointIds[ i ], &jointConf );
			if ( jointConf > .9f )
			{
				mRibbonManager.createRibbon( jointId );
				mRibbonManager.setActive( jointId, true );
				mRibbonManager.update( jointId, joint3d );
			}
			else
			if ( mRibbonManager.findRibbon( jointId ) != RibbonRef() )
			{
				mRibbonManager.setActive( jointId, false );
			}
		}
	}
}

void RibbonEffect::draw()
{
	gl::setViewport( getBounds() );
	gl::setMatrices( mMayaCam.getCamera() );

	gl::enableDepthRead();
	gl::enableDepthWrite();

	// setup light 0
	/*
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setDirection( mLightDirection );
	light.setAmbient( mLightAmbient );
	light.setDiffuse( mLightDiffuse );
	light.setSpecular( mLightSpecular );
	light.enable();

	gl::enable( GL_CULL_FACE );
	gl::enable( GL_LIGHTING );
	*/

	Vec3f cameraRight, cameraUp;
	mMayaCam.getCamera().getBillboardVectors( &cameraRight, &cameraUp );
	Vec3f cameraDir = cameraUp.cross( cameraRight );

	gl::color( mMaterialDiffuse );
	mRibbonManager.draw( cameraDir );

	/*
	gl::disable( GL_LIGHTING );
	gl::disable( GL_CULL_FACE );
	*/
	gl::disableDepthRead();
	gl::disableDepthWrite();
}

void RibbonEffect::drawControl()
{
	mParams.draw();
	mRibbonManager.drawControl();
}

void RibbonEffect::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void RibbonEffect::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void RibbonEffect::shutdown()
{
}
