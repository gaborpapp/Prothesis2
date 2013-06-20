#include <vector>

#include "cinder/gl/gl.h"

#include "GlobalData.h"
#include "NIDebugDraw.h"

using namespace ci;

void NIDebugDraw::drawJoints()
{
	GlobalData &gd = GlobalData::get();
	if ( !gd.mNI )
		return;

	const XnSkeletonJoint jointIds[] = {
		XN_SKEL_HEAD, XN_SKEL_NECK, XN_SKEL_TORSO,
		XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,
		XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,
		XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,
		XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT };

	gl::disableDepthRead();
	gl::disableDepthWrite();

	gl::disable( GL_LIGHTING );
	gl::color( Color::white() );
	std::vector< unsigned > users = gd.mNIUserTracker.getUsers();
	if ( !users.empty() )
	{
		// one user only
		unsigned userId = users[ 0 ];

		for ( int i = 0; i < sizeof( jointIds ) / sizeof( jointIds[ 0 ] ); i++ )
		{
			float jointConf;
			Vec3f joint3d = gd.mNIUserTracker.getJoint3d( userId, jointIds[ i ], &jointConf );
			if ( jointConf > .9f )
			{
				gl::drawSphere( joint3d, 10 );
			}
		}
	}
}

void NIDebugDraw::drawLines()
{
	GlobalData &gd = GlobalData::get();
	if ( !gd.mNI )
		return;

	const XnSkeletonJoint segmentIds[][ 2 ] = {
		{ XN_SKEL_LEFT_HAND, XN_SKEL_LEFT_ELBOW },
		{ XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_SHOULDER },
		{ XN_SKEL_LEFT_SHOULDER, XN_SKEL_NECK },
		{ XN_SKEL_RIGHT_SHOULDER, XN_SKEL_NECK },
		{ XN_SKEL_RIGHT_HAND, XN_SKEL_RIGHT_ELBOW },
		{ XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_SHOULDER },
		{ XN_SKEL_HEAD, XN_SKEL_NECK },
		{ XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO },
		{ XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO },
		{ XN_SKEL_TORSO, XN_SKEL_LEFT_HIP },
		{ XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP },
		{ XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP },
		{ XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE },
		{ XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE },
		{ XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT },
		{ XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT } };

	gl::disableDepthRead();
	gl::disableDepthWrite();

	gl::disable( GL_LIGHTING );
	glLineWidth( 3 );
	gl::color( Color::white() );
	std::vector< unsigned > users = gd.mNIUserTracker.getUsers();
	if ( !users.empty() )
	{
		// one user only
		unsigned userId = users[ 0 ];

		for ( int i = 0; i < sizeof( segmentIds ) / sizeof( segmentIds[ 0 ] ); i++ )
		{
			float jointConf0, jointConf1;
			Vec3f joint03d = gd.mNIUserTracker.getJoint3d( userId, segmentIds[ i ][ 0 ], &jointConf0 );
			Vec3f joint13d = gd.mNIUserTracker.getJoint3d( userId, segmentIds[ i ][ 1 ], &jointConf1 );
			if ( ( jointConf0 > .9f ) && ( jointConf1 > .9f ) )
			{
				gl::drawLine( joint03d, joint13d );
			}
		}
	}
	glLineWidth( 1 );
}
