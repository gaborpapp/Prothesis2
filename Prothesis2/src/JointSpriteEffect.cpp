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
#include "NIDebugDraw.h"
#include "JointSpriteEffect.h"

using namespace boost::assign;
using namespace ci;
using namespace std;

#define XN_SKEL_MIN XN_SKEL_HEAD // NOTE: starts at 1
#define XN_SKEL_MAX XN_SKEL_RIGHT_FOOT
#define XN_SKEL_NUM ( XN_SKEL_RIGHT_FOOT + 1 )

void JointSpriteEffect::setup()
{
	loadSprites( "JointSprites" );
	mJointSprites.resize( XN_SKEL_NUM + XN_SKEL_HEAD );

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

		mParams.addPersistentParam( jointNames[ i ] + " sprite", mSpriteNames, &mJointSprites[ jointId ].spriteId, 0 );
		if ( mJointSprites[ jointId ].spriteId >= mSpriteNames.size() )
			mJointSprites[ jointId ].spriteId = 0;
	}

	mParams.addSeparator();
	mParams.addPersistentParam( "Sprite scale X", &mSpriteScale.x, 1.f, "min=0 step=.01" );
	mParams.addPersistentParam( "Sprite scale Y", &mSpriteScale.y, 1.f, "min=0 step=.01" );
	mParams.addPersistentParam( "Sprite rotation", &mSpriteRotation, 0.f, "min=0 step=.5" );
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

		struct CurrentJointSprite
		{
			CurrentJointSprite( const XnSkeletonJoint &jId, const Vec3f &p ) :
				jointId( jId ), pos( p ) {}

			XnSkeletonJoint jointId;
			Vec3f pos;
			float cameraZ;
		};
		vector < CurrentJointSprite > jointSprites;

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
						jointSprites.push_back( CurrentJointSprite( jointId, pos ) );
						Vec3f tpos = modelview * pos;
						jointSprites.back().cameraZ = tpos.z;
						mJointSprites[ jointId ].lastPos = pos;
						mJointSprites[ jointId ].lastSeen = currentTime;
					}
					else
					if ( ( currentTime - mJointSprites[ jointId ].lastSeen ) < mJointDisappearThr )
					{
						jointSprites.push_back( CurrentJointSprite( jointId, mJointSprites[ jointId ].lastPos ) );
						Vec3f tpos = modelview * mJointSprites[ jointId ].lastPos;
						jointSprites.back().cameraZ = tpos.z;
					}
				}
			}
		}

		// sort and draw sprites
		std::sort( jointSprites.begin(), jointSprites.end(),
				[&]( const CurrentJointSprite &s0, const CurrentJointSprite &s1 )
				{ return ( s0.cameraZ < s1.cameraZ ); } );
		gl::enableAlphaBlending();
		gl::enable( GL_TEXTURE_2D );
		gl::color( Color::white() );
		for ( auto spriteIt = jointSprites.cbegin(); spriteIt != jointSprites.cend(); ++spriteIt )
		{
			gl::Texture jointTxt = mSprites[ mJointSprites[ spriteIt->jointId ].spriteId ];
			if ( !jointTxt )
				continue;
			jointTxt.bind();
			gl::drawBillboard( spriteIt->pos, mSpriteScale * Vec2f( jointTxt.getSize() ), mSpriteRotation, cameraRight, cameraUp );
			jointTxt.unbind();
		}
		gl::disable( GL_TEXTURE_2D );
		gl::disableAlphaBlending();
	}

	if ( GlobalData::get().mNIDebugJoints )
		NIDebugDraw::drawJoints();
	if ( GlobalData::get().mNIDebugLines )
		NIDebugDraw::drawLines();
}

void JointSpriteEffect::loadSprites( const fs::path &relativeDir )
{
	fs::path dataPath = app::getAssetPath( relativeDir );

	mSprites.push_back( gl::Texture() );
	mSpriteNames.push_back( "none" );

	if ( dataPath.empty() )
	{
		app::console() << "Could not find sprite directory assets/" << relativeDir.string() << std::endl;
		return;
	}

	for ( fs::directory_iterator it( dataPath ); it != fs::directory_iterator(); ++it )
	{
		if ( fs::is_regular_file(*it) )
		{
			try
			{
				gl::Texture t = loadImage( app::loadAsset( relativeDir / it->path().filename() ) );
				mSprites.push_back( t );
				mSpriteNames.push_back( it->path().filename().string() );
			}
			catch ( const ImageIoException &exc  )
			{
				app::console() << "Unable to load image " << it->path() << ": " << exc.what() << std::endl;
			}
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

