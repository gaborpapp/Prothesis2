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

using namespace boost::assign;
using namespace ci;
using namespace ci::app;
using namespace std;

void RibbonEffect::setup()
{
	mParams = mndl::params::PInterfaceGl( GlobalData::get().mControlWindow, "Ribbon Effect", Vec2i( 200, 310 ), Vec2i( 518, 16 ) );
	mParams.addPersistentSizeAndPosition();
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

	mParams.addText( "Ribbon" );
	mParams.addPersistentParam( "Ribbon length", &mRibbonMaxLength, 32, "min=10 max=1000" );
	mParams.addPersistentParam( "Ribbon width", &mRibbonWidth, 16.0f, "min= 0.1f max=100.0 step=0.1" );
	mParams.addPersistentParam( "Joint disappearance threshold", &mJointDisappearanceThr, 0.5f, "min= 0.1f max=60.0 step=0.1" );
	mParams.addSeparator();
	mParams.addPersistentParam( "Stiffness", &mK, 0.06f, "min=0.01 max=0.2 step=0.01" );
	mParams.addPersistentParam( "Damping", &mDamping, 0.7f, "min=0 max=0.99 step=0.01" );
	mParams.addSeparator();

	vector< string > jointNames;
	jointNames += "head", "neck", "torso", "left shoulder",
		"left elbow", "left hand", "right shoulder", "right elbow", "right hand",
		"left hip", "left knee", "left foot", "right hip", "right knee", "right foot";
	const XnSkeletonJoint jointIds[] = {
		XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
		XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
		XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
		XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
		XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };

	for( size_t i = 0; i < jointNames.size(); i++ )
	{
		XnSkeletonJoint jointId = jointIds[ i ];
		bool defaultValue = ( jointId == XN_SKEL_LEFT_HAND ) || ( jointId == XN_SKEL_RIGHT_HAND );

		mParams.addPersistentParam( jointNames[ i ] + " active", &mRibbonActive[ jointId ], defaultValue );
	}

	mParams.addSeparator();
	mParams.addButton( "Reset", [&]() { mRibbonManager.clear(); } );

	CameraPersp cam;
	cam.setPerspective( 60, getAspectRatio(), 1, 15000 );
	cam.setEyePoint( Vec3f( 0, 0, 0 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 800 ) );
	mMayaCam.setCurrentCam( cam );

	try
	{
		mPhongShader = gl::GlslProg( app::loadResource( RES_PHONG_DIRECTIONAL_VERT ),
									 app::loadResource( RES_PHONG_DIRECTIONAL_FRAG ) );
	}
	catch ( gl::GlslProgCompileExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}
}

void RibbonEffect::update()
{
	GlobalData &gd = GlobalData::get();
	if ( !gd.mNI )
		return;

	// update ribbon parameters
	Ribbon::setMaxLength( mRibbonMaxLength );
	Ribbon::setWidth( mRibbonWidth );
	Ribbon::setStiffness( mK );
	Ribbon::setDamping( mDamping );

	// this translates the params optionmenu index to the skeleton joint id, the order is important
	const XnSkeletonJoint jointIds[] = {
		XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
		XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
		XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
		XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
		XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };
	static double jointLastSeen[ sizeof( jointIds ) / sizeof( jointIds[ 0 ] ) ] = { 0., };

	double time = app::getElapsedSeconds();
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
			if ( ( jointConf > .9f ) && mRibbonActive[ jointId ] )
			{
				RibbonRef ribbon = mRibbonManager.createRibbon( jointId );
				ribbon->addPos( joint3d );
				jointLastSeen[ i ] = time;
			}
			else
			if ( ( ( time - jointLastSeen[ i ] ) > mJointDisappearanceThr ) || ( !mRibbonActive[ jointId ] ) )
			{
				mRibbonManager.detachRibbon( jointId );
			}
		}
	}

	mRibbonManager.update();
}

void RibbonEffect::draw()
{
	gl::setViewport( getBounds() );
	gl::setMatrices( mMayaCam.getCamera() );

	gl::enableDepthRead();
	gl::enableDepthWrite();

	Vec3f cameraRight, cameraUp;
	mMayaCam.getCamera().getBillboardVectors( &cameraRight, &cameraUp );
	Vec3f cameraDir = cameraUp.cross( cameraRight );

	// setup light 0
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	Quatf q( -Vec3f::zAxis(), mLightDirection );
	light.setDirection( -cameraDir * q );
	//light.setDirection( mLightDirection );
	light.setAmbient( mLightAmbient );
	light.setDiffuse( mLightDiffuse );
	light.setSpecular( mLightSpecular );
	light.enable();

	gl::enable( GL_LIGHTING );

	gl::Material material( mMaterialAmbient, mMaterialDiffuse, mMaterialSpecular );
	material.setShininess( mMaterialShininess );
	material.apply();

	if ( mPhongShader )
		mPhongShader.bind();
	mRibbonManager.draw( cameraDir );
	if ( mPhongShader )
		mPhongShader.unbind();

	gl::disable( GL_LIGHTING );
	gl::disableDepthRead();
	gl::disableDepthWrite();
}

void RibbonEffect::drawControl()
{
	mParams.draw();
}

void RibbonEffect::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void RibbonEffect::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}
