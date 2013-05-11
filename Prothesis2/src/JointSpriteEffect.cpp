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
#include "cinder/ImageIo.h"

#include "GlobalData.h"
#include "JointSpriteEffect.h"

using namespace boost::assign;
using namespace ci;
using namespace std;

#define XN_SKEL_MIN XN_SKEL_HEAD // NOTE: starts at 1
#define XN_SKEL_MAX XN_SKEL_RIGHT_FOOT
#define XN_SKEL_NUM ( XN_SKEL_RIGHT_FOOT + 1 )

void JointSpriteEffect::setup()
{
	mJointTextures.resize( XN_SKEL_NUM + XN_SKEL_HEAD );
	mJointTextureFilenames.resize( XN_SKEL_NUM + XN_SKEL_HEAD );
	mLastJointPositions.resize( XN_SKEL_NUM + XN_SKEL_HEAD );

	mParams = mndl::params::PInterfaceGl( GlobalData::get().mControlWindow,
			"JointSprite Effect", Vec2i( 490, 310 ), Vec2i( 734, 340 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addPersistentParam( "Joint disappear thr", &mJointDisappearThr, .7f, "min=0.f max=10.f step=.01" );
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

		mParams.addPersistentParam( jointNames[ i ] + " filename", &mJointTextureFilenames[ jointId ], "", "", true );
		mParams.addButton( "load " + jointNames[ i ] + " image", std::bind( &JointSpriteEffect::loadJointTexture, this, jointId ) );
		mParams.addButton( "reset " + jointNames[ i ] + " image", [&, jointId]() {
				mJointTextureFilenames[ jointId ] = "";
				mJointTextures[ jointId ] = gl::Texture();
			} );
		mParams.addSeparator();

		if ( mJointTextureFilenames[ jointId ] != "" )
		{
			try
			{
				mJointTextures[ jointId ] = gl::Texture( loadImage( mJointTextureFilenames[ jointId ] ) );
			}
			catch ( const ImageIoException &exc  )
			{
				app::console() << "Unable to load image " << mJointTextureFilenames[ jointId ] << " " << exc.what() << std::endl;
			}
		}
	}

	mParams.addSeparator();

	CameraPersp cam;
	cam.setPerspective( 60, getAspectRatio(), 1, 15000 );
	cam.setEyePoint( Vec3f( 0, 0, 0 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 800 ) );
	mMayaCam.setCurrentCam( cam );
}

void JointSpriteEffect::update()
{
}

void JointSpriteEffect::draw()
{
	gl::setViewport( getBounds() );
	gl::setMatrices( mMayaCam.getCamera() );

	Vec3f cameraRight, cameraUp;
	mMayaCam.getCamera().getBillboardVectors( &cameraRight, &cameraUp );

	GlobalData &gd = GlobalData::get();
	if ( gd.mNI )
	{
		// this translates the params optionmenu index to the skeleton joint id, the order is important
		const XnSkeletonJoint jointIds[] = {
			XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
			XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
			XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
			XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
			XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };

		struct JointSprite
		{
			JointSprite( const XnSkeletonJoint &jId, const Vec3f &p ) :
				jointId( jId ), pos( p ) {}

			XnSkeletonJoint jointId;
			Vec3f pos;
			float z;
		};
		vector < JointSprite > jointSprites;

		double currentTime = app::getElapsedSeconds();
		// get joint 3d coordinates
		vector< unsigned > users = gd.mNIUserTracker.getUsers();
		if ( !users.empty() )
		{
			const Matrix44f &modelview = mMayaCam.getCamera().getModelViewMatrix();

			for ( auto userIt = users.cbegin(); userIt != users.end(); ++userIt )
			{
				unsigned userId = *userIt;

				for ( int i = 0; i < sizeof( jointIds ) / sizeof( jointIds[ 0 ] ); i++ )
				{
					XnSkeletonJoint jointId = jointIds[ i ];
					float jointConf;
					Vec3f pos = gd.mNIUserTracker.getJoint3d( userId, jointIds[ i ], &jointConf );
					if ( jointConf > .9f )
					{
						jointSprites.push_back( JointSprite( jointId, pos ) );
						Vec3f tpos = modelview * pos;
						jointSprites.back().z = tpos.z;
						mLastJointPositions[ jointId ].pos = pos;
						mLastJointPositions[ jointId ].lastSeen = currentTime;
					}
					else
					if ( ( currentTime - mLastJointPositions[ jointId ].lastSeen ) < mJointDisappearThr )
					{
						jointSprites.push_back( JointSprite( jointId, mLastJointPositions[ jointId ].pos ) );
						Vec3f tpos = modelview * mLastJointPositions[ jointId ].pos;
						jointSprites.back().z = tpos.z;
					}
				}
			}
		}

		// sort and draw sprites
		std::sort( jointSprites.begin(), jointSprites.end(),
				[&]( const JointSprite &s0, const JointSprite &s1 )
				{ return ( s0.z < s1.z ); } );
		gl::enableAlphaBlending();
		gl::enable( GL_TEXTURE_2D );
		gl::color( Color::white() );
		for ( auto spriteIt = jointSprites.cbegin(); spriteIt != jointSprites.cend(); ++spriteIt )
		{
			gl::Texture jointTxt = mJointTextures[ spriteIt->jointId ];
			if ( !jointTxt )
				continue;
			jointTxt.bind();
			gl::drawBillboard( spriteIt->pos, Vec2f( jointTxt.getSize() ), 0.f, cameraRight, cameraUp );
			jointTxt.unbind();
		}
		gl::disable( GL_TEXTURE_2D );
		gl::disableAlphaBlending();
	}
}

void JointSpriteEffect::loadJointTexture( const XnSkeletonJoint &jointId )
{
	ci::fs::path appPath( ci::app::getAppPath() );
#ifdef CINDER_MAC
	appPath = appPath.parent_path();
#endif
	ci::fs::path imagePath = ci::app::getOpenFilePath( appPath );

	if ( !imagePath.empty() )
	{
		try
		{
			mJointTextures[ jointId ] = gl::Texture( loadImage( imagePath ) );
			mJointTextureFilenames[ jointId ] = imagePath.string();
		}
		catch ( const ImageIoException &exc  )
		{
			ci::app::console() << "Unable to load image " << imagePath << " " << exc.what() << std::endl;
		}
	}
}

void JointSpriteEffect::drawControl()
{
	mParams.draw();
}

void JointSpriteEffect::mouseDown( app::MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void JointSpriteEffect::mouseDrag( app::MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

