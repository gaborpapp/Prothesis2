/*
 Copyright (C) 2013 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
#include "cinder/TriMesh.h"

#include "CiNI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SkelMeshApp : public AppBasic
{
	public:
		void prepareSettings( Settings *settings );
		void setup();

		void keyDown( KeyEvent event );
		void mouseDown( MouseEvent event );
		void mouseDrag( MouseEvent event );
		void resize();

		void update();
		void draw();

	private:
		params::InterfaceGl mParams;

		float mFps;
		bool mVerticalSyncEnabled = false;

		mndl::ni::OpenNI mNI;
		mndl::ni::UserTracker mNIUserTracker;
		gl::Texture mColorTexture;

		MayaCamUI mMayaCam;

#define XN_SKEL_MIN XN_SKEL_HEAD // NOTE: starts at 1
#define XN_SKEL_MAX XN_SKEL_RIGHT_FOOT
#define XN_SKEL_NUM ( XN_SKEL_RIGHT_FOOT + 1 )

		struct Position
		{
			Position()
			{
				for ( int i = 0; i < XN_SKEL_NUM; i++ )
				{
					mConf[ i ] = 0.f;
				}
			}
			Vec3f mJoints[ XN_SKEL_NUM ];
			float mConf[ XN_SKEL_NUM ];
		};

		vector< Position > mPositions;
		int mMaxPositions;
		int mStoredPositions;
		int mCurrentPosition;

		void updateSkeleton();
};

void SkelMeshApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void SkelMeshApp::setup()
{
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 300 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addParam( "Vertical sync", &mVerticalSyncEnabled );

	try
	{
		//mNI = ni::OpenNI( ni::OpenNI::Device() );
		fs::path path = getAppPath();
#ifdef CINDER_MAC
		path /= "/../";
#endif
		path /= "captured-130404.oni";
		mNI = mndl::ni::OpenNI( path );
	}
	catch ( ... )
	{
		console() << "Could not open Kinect" << endl;
		quit();
	}

	mNI.start();
	mNIUserTracker = mNI.getUserTracker();
	mNIUserTracker.setSmoothing( .7 );

	CameraPersp cam;
	cam.setPerspective( 60, getWindowAspectRatio(), 1, 15000 );
	cam.setEyePoint( Vec3f( 0, 0, 0 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 800 ) );
	mMayaCam.setCurrentCam( cam );

	mStoredPositions = 0;
	mCurrentPosition = 0;
	mMaxPositions = 128;
	mPositions.resize( mMaxPositions );
	/*
	for ( int i = 0; i < mMaxPositions; i++ )
	{
		mPositions[ i ] = Position();
	}
	*/
}

void SkelMeshApp::update()
{
	mFps = getAverageFps();

	if ( mVerticalSyncEnabled != gl::isVerticalSyncEnabled() )
		gl::enableVerticalSync( mVerticalSyncEnabled );

	if ( mNI.checkNewVideoFrame() )
		mColorTexture = mNI.getVideoImage();

	updateSkeleton();
}

void SkelMeshApp::updateSkeleton()
{
	const XnSkeletonJoint jointIds[] = {
		XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
		XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
		XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
		XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
		XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };

	vector< unsigned > users = mNIUserTracker.getUsers();
	if ( !users.empty() )
	{
		// one user only
		unsigned userId = users[ 0 ];

		for ( int i = 0; i < sizeof( jointIds ) / sizeof( jointIds[ 0 ] ); i++ )
		{
			XnSkeletonJoint jointId = jointIds[ i ];
			mPositions[ mCurrentPosition ].mJoints[ jointId ] =
				mNIUserTracker.getJoint3d( userId, jointIds[i],
				&mPositions[ mCurrentPosition ].mConf[ jointId ] );
			console() << i << " " << mPositions[ mCurrentPosition ].mJoints[ jointId ] << " " <<
				mPositions[ mCurrentPosition ].mConf[ jointId ] << endl;

		}
		mCurrentPosition++;
		mStoredPositions = math< int >::min( mStoredPositions + 1, mMaxPositions );
		if ( mCurrentPosition >= mMaxPositions )
			mCurrentPosition = 0;
	}
}

void SkelMeshApp::draw()
{
	gl::clear( Color::black() );
	gl::setViewport( getWindowBounds() );
	gl::setMatrices( mMayaCam.getCamera() );

	/*
	if ( mColorTexture )
		gl::draw( mColorTexture );
	*/
	gl::enableDepthRead();
	gl::enableDepthWrite();

	/*
	for ( int ii = mCurrentPosition - mStoredPositions; ii < mCurrentPosition; ii++ )
	{
		int i = ii;
		if ( i < 0 )
			i += mMaxPositions;
		for ( int k = 0; k < XN_SKEL_NUM; k++ )
		{
			if ( mPositions[ i ].mConf[ k ] > .9f )
			{
				gl::drawSphere( mPositions[ i ].mJoints[ k ], 10.f );
			}
		}
	}
	*/
	XnSkeletonJoint edges[][2] = {
		{ XN_SKEL_HEAD, XN_SKEL_NECK },
		{ XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER },
		{ XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW },
		{ XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND },
		{ XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER },
		{ XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW },
		{ XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND },
		{ XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO },
		{ XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO },
		{ XN_SKEL_TORSO, XN_SKEL_LEFT_HIP },
		{ XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP },
		{ XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE },
		{ XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT },
		{ XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP },
		{ XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE },
		{ XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT } };

	TriMesh mesh;
	size_t currentId = 0;
	for ( int e = 0; e < sizeof( edges ) / sizeof( edges[ 0 ] ); e++ )
	{
		for ( int ii = mCurrentPosition - mStoredPositions; ii < mCurrentPosition; ii++ )
		{
			int current = ii;
			int last = ii - 1;
			if ( current < 0 )
				current += mMaxPositions;
			if ( last < 0 )
				last += mMaxPositions;

			XnSkeletonJoint id0 = edges[ e ][ 0 ];
			XnSkeletonJoint id1 = edges[ e ][ 1 ];

			if ( ( mPositions[ current ].mConf[ id0 ] > .9f ) &&
				 ( mPositions[ current ].mConf[ id1 ] > .9f ) &&
				 ( mPositions[ last ].mConf[ id0 ] > .9f ) &&
				 ( mPositions[ last ].mConf[ id1 ] > .9f ) )
			{
				mesh.appendVertex( mPositions[ current ].mJoints[ id0 ] );
				mesh.appendVertex( mPositions[ current ].mJoints[ id1 ] );
				mesh.appendVertex( mPositions[ last ].mJoints[ id1 ] );
				mesh.appendVertex( mPositions[ last ].mJoints[ id0 ] );
				mesh.appendTriangle( currentId, currentId + 1, currentId + 2 );
				mesh.appendTriangle( currentId, currentId + 2, currentId + 3 );
				currentId += 4;
				Vec3f edge0 = mPositions[ current ].mJoints[ id0 ] -
							  mPositions[ current ].mJoints[ id1 ];
				Vec3f edge1 = mPositions[ current ].mJoints[ id0 ] -
							  mPositions[ last ].mJoints[ id1 ];
				Vec3f normal = edge0.cross( edge1 ).normalized();
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
			}
		}
	}
	gl::enableWireframe();
	gl::draw( mesh );
	gl::disableWireframe();


	params::InterfaceGl::draw();
}

void SkelMeshApp::keyDown( KeyEvent event )
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
			mParams.show( !mParams.isVisible() );
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

		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		default:
			break;
	}
}

void SkelMeshApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void SkelMeshApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void SkelMeshApp::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

CINDER_APP_BASIC( SkelMeshApp, RendererGl )

