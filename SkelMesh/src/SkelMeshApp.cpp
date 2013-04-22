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
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Material.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
#include "cinder/TriMesh.h"

#include "CiNI.h"
#include "mndlkit/params/PParams.h"

#include "Resources.h"

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
		void shutdown();

		void update();
		void draw();

	private:
		mndl::params::PInterfaceGl mParams;
		mndl::params::PInterfaceGl mEdgeParams;

		float mFps;
		bool mVerticalSyncEnabled;

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

		Vec3f mLightDirection;
		Color mLightAmbient;
		Color mLightDiffuse;
		Color mLightSpecular;

		Color mMaterialAmbient;
		Color mMaterialDiffuse;
		Color mMaterialSpecular;
		float mMaterialShininess;

		gl::GlslProg mPhongShader;

		void addEdge( int edgeId );
		void rebuildEdgeParams();
		void removeEdgeFromParams( int edgeId );

		int mEdgeNum = 0;
		const int MAX_EDGE_NUM = 128;

		struct Edge
		{
			Edge() : mJoint0( 0 ), mJoint1( 0 ) {}
			int mJoint0, mJoint1;
		};
		vector< Edge > mEdges;

		void loadConfig( const fs::path &fname );
		void saveConfig();
		fs::path mConfigFile;
};

void SkelMeshApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void SkelMeshApp::setup()
{
	mEdges.resize( MAX_EDGE_NUM );

	mndl::params::PInterfaceGl::load( "params.xml" );
	mParams = mndl::params::PInterfaceGl( "Parameters", Vec2i( 200, 300 ), Vec2i( 16, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addPersistentParam( "Vertical sync", &mVerticalSyncEnabled, false );
	mParams.addSeparator();
	mParams.addPersistentParam( "Light direction", &mLightDirection, Vec3f( 1.42215264, -1.07486057, -0.842143714 ) );
	mParams.addPersistentParam( "Light ambient", &mLightAmbient, Color::black() );
	mParams.addPersistentParam( "Light diffuse", &mLightDiffuse, Color::white() );
	mParams.addPersistentParam( "Light specular", &mLightDiffuse, Color::white() );
	mParams.addSeparator();
	mParams.addPersistentParam( "Material ambient", &mMaterialAmbient, Color::black() );
	mParams.addPersistentParam( "Material diffuse", &mMaterialDiffuse, Color::gray( .5f ) );
	mParams.addPersistentParam( "Material specular", &mMaterialSpecular, Color::white() );
	mParams.addPersistentParam( "Material shininess", &mMaterialShininess, 50.f, "min=0 max=10000 step=.5" );
	mParams.addSeparator();
	mParams.addButton( "Reset", [&]() { mStoredPositions = 0; mCurrentPosition = 0; } );

	mEdgeParams = mndl::params::PInterfaceGl( "Edges", Vec2i( 200, 300 ), Vec2i( 232, 16 ) );
	mEdgeParams.addPersistentSizeAndPosition();

	loadConfig( "config.xml" );
	rebuildEdgeParams();

	try
	{
		mPhongShader = gl::GlslProg( loadResource( RES_PHONG_DIRECTIONAL_VERT ),
									 loadResource( RES_PHONG_DIRECTIONAL_FRAG ) );
	}
	catch ( gl::GlslProgCompileExc &exc )
	{
		console() << exc.what() << endl;
	}

	try
	{
//#define KINECT_USE_RECORDING
#ifndef KINECT_USE_RECORDING
		mNI = mndl::ni::OpenNI( mndl::ni::OpenNI::Device() );
#else
		fs::path path = getAppPath();
#ifdef CINDER_MAC
		path /= "/../";
#endif
		path /= "captured-130404.oni";
		mNI = mndl::ni::OpenNI( path );
#endif
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
	mMaxPositions = 256;
	mPositions.resize( mMaxPositions );
	/*
	for ( int i = 0; i < mMaxPositions; i++ )
	{
		mPositions[ i ] = Position();
	}
	*/
}

void SkelMeshApp::addEdge( int edgeId )
{
	vector< string> jointNames = { "head", "neck", "torso", "left shoulder",
		"left elbow", "left hand", "right shoulder", "right elbow", "right hand",
		"left hip", "left knee", "left foot", "right hip", "right knee", "right foot" };

	string edgeName = "Edge " + toString< int >( edgeId );
	mEdgeParams.addText( edgeName );
	mEdgeParams.addParam( edgeName + " joint 0", jointNames, &mEdges[ edgeId ].mJoint0 );
	mEdgeParams.addParam( edgeName + " joint 1", jointNames, &mEdges[ edgeId ].mJoint1 );
	mEdgeParams.addButton( "Remove " + edgeName, [ this, edgeId ]() { removeEdgeFromParams( edgeId ); } );
	mEdgeParams.addSeparator();
}

void SkelMeshApp::rebuildEdgeParams()
{
	if ( mEdgeNum >= MAX_EDGE_NUM )
	{
		mEdgeNum = MAX_EDGE_NUM - 1;
		return;
	}

	mEdgeParams.clear();
	for ( int i = 0; i < mEdgeNum; i++ )
		addEdge( i );

	mEdgeParams.addButton( "Add edge", [this]()
			{
				mEdges[ mEdgeNum ] = Edge();
				mEdgeNum++;
				rebuildEdgeParams();
			} );
}

void SkelMeshApp::removeEdgeFromParams( int edgeId )
{
	mEdgeNum--;
	// copy parameter values
	for ( int i = edgeId; i < mEdgeNum; i++ )
		mEdges[ i ] = mEdges[ i + 1 ];

	rebuildEdgeParams();
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
		for ( int ii = mCurrentPosition - mStoredPositions + 1; ii < mCurrentPosition; ii++ )
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

				// backface
				mesh.appendVertex( mPositions[ current ].mJoints[ id0 ] );
				mesh.appendVertex( mPositions[ current ].mJoints[ id1 ] );
				mesh.appendVertex( mPositions[ last ].mJoints[ id1 ] );
				mesh.appendVertex( mPositions[ last ].mJoints[ id0 ] );
				mesh.appendTriangle( currentId, currentId + 2, currentId + 1 );
				mesh.appendTriangle( currentId, currentId + 3, currentId + 2 );
				currentId += 4;
				normal = -normal;
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
				mesh.appendNormal( normal );
			}
		}
	}

	// setup light 0
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setDirection( mLightDirection );
	light.setAmbient( mLightAmbient );
	light.setDiffuse( mLightDiffuse );
	light.setSpecular( mLightSpecular );
	//light.setShadowParams( 60.0f, 5.0f, lightPosition.length() * 1.5f );
	light.enable();

	gl::Material material( mMaterialAmbient, mMaterialDiffuse, mMaterialSpecular );
	material.setShininess( mMaterialShininess );
	material.apply();

	gl::enable( GL_CULL_FACE );
	gl::enable( GL_LIGHTING );
	if ( mPhongShader )
		mPhongShader.bind();
	if ( mesh.getNumVertices() )
		gl::draw( mesh );
	if ( mPhongShader )
		mPhongShader.unbind();
	gl::disable( GL_LIGHTING );

	/*
	{
		const std::vector< Vec3f > &v = mesh.getVertices();
		const std::vector< Vec3f > &n = mesh.getNormals();
		gl::color( Color( 1, 0, 0 ) );
		for ( size_t i = 0; i < n.size(); i++ )
		{
			gl::drawVector( v[ i ], v[ i ] + 100 * n[ i ] );
		}
	}
	*/

	mParams.draw();
	mEdgeParams.draw();
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

void SkelMeshApp::shutdown()
{
	mndl::params::PInterfaceGl::save();
	saveConfig();
}

void SkelMeshApp::loadConfig( const fs::path &fname )
{
	fs::path configXml( app::getAssetPath( fname ));
	if ( configXml.empty() )
	{
#if defined( CINDER_MAC )
		fs::path assetPath( app::App::getResourcePath() / "assets" );
#else
		fs::path assetPath( app::App::get()->getAppPath() / "assets" );
#endif
		createDirectories( assetPath );
		configXml = assetPath / fname ;
	}

	mConfigFile = configXml;
	if ( !fs::exists( configXml ) )
		return;

	XmlTree config( loadFile( configXml ) );

	XmlTree edges = config.getChild( "edges" );
	mEdgeNum = edges.getAttributeValue< int >( "num" );
	int edgeId = 0;
	for ( XmlTree::Iter pit = config.begin( "edges/edge"); pit != config.end(); ++pit, edgeId++ )
	{
		mEdges[ edgeId ].mJoint0 = pit->getAttributeValue< int >( "joint0" );
		mEdges[ edgeId ].mJoint1 = pit->getAttributeValue< int >( "joint1" );
	}
}

void SkelMeshApp::saveConfig()
{
	XmlTree config( "edges", "" );

	config.setAttribute( "num", mEdgeNum );
	for ( int i = 0; i < mEdgeNum; i++ )
	{
		XmlTree t = XmlTree( "edge", "" );
		t.setAttribute( "joint0", mEdges[ i ].mJoint0 );
		t.setAttribute( "joint1", mEdges[ i ].mJoint1 );
		config.push_back( t );
	}
	config.write( writeFile( mConfigFile ) );
}

CINDER_APP_BASIC( SkelMeshApp, RendererGl )

