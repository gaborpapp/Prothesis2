#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>

#include "cinder/app/App.h"
#include "cinder/ip/grayscale.h"
#include "AntTweakBar.h"
#include "NIUser.h"
using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mndl::ni;

namespace cinder
{

/*
visualize kinect joints

        o
        |
o--o--o---o--o--o
       \ /
        o
       / \
      |   |
      o   o
      |   |
      o   o
*/

User::User( UserManager *userManager )
: mUserManager( userManager )
{
}

void User::clearPoints()
{
	mJointPositions.clear();
}

void User::update( XnSkeletonJoint jointId, Vec3f pos )
{
	mJointPositions.insert( pair< XnSkeletonJoint, Vec3f >( jointId, pos ));

	mRibbonManager.setActive( jointId , mUserManager->getRibbonActive( jointId ));
	mRibbonManager.update( jointId , pos );

	if( jointId == mUserManager->mJointRef )
		mPosRef = pos;
}

void User::draw( const ci::Vec3f& cameraDir )
{
	mRibbonManager.draw( cameraDir );
}

void User::addRibbon( XnSkeletonJoint jointId )
{
	mRibbonManager.createRibbon( jointId );
}

void User::clearRibbons()
{
	mRibbonManager.clear();
}

void User::drawBody()
{
	if( mUserManager->mJointShow )
		drawJoints();
	if( mUserManager->mLineShow )
		drawLines();
}

void User::drawJoints()
{
	gl::color( mUserManager->mJointColor );
	float sc = mUserManager->mOutputRect.getWidth() / 640.0f;
	float scaledJointSize = mUserManager->mJointSize * sc;
	RectMapping mapping( mUserManager->mOutputRect, mUserManager->mSourceBounds );

	for( JointPositions::const_iterator it = mJointPositions.begin(); it != mJointPositions.end(); ++it )
	{
		XnSkeletonJoint jointId = it->first;

		if( jointId == XN_SKEL_NECK
		 || jointId == XN_SKEL_LEFT_HIP
		 || jointId == XN_SKEL_RIGHT_HIP )
			continue;

		Vec3f pos = it->second;
		gl::drawSphere( pos, scaledJointSize );
	}
	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawLines()
{
	gl::color( mUserManager->mJointColor );

	drawLine( XN_SKEL_LEFT_HAND     , XN_SKEL_LEFT_SHOULDER  );
	drawLine( XN_SKEL_LEFT_SHOULDER , XN_SKEL_NECK           );
	drawLine( XN_SKEL_RIGHT_SHOULDER, XN_SKEL_NECK           );
	drawLine( XN_SKEL_RIGHT_HAND    , XN_SKEL_RIGHT_SHOULDER );
	drawLine( XN_SKEL_HEAD          , XN_SKEL_NECK           );
	drawLine( XN_SKEL_LEFT_SHOULDER , XN_SKEL_TORSO          );
	drawLine( XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO          );
	drawLine( XN_SKEL_TORSO         , XN_SKEL_LEFT_HIP       );
	drawLine( XN_SKEL_TORSO         , XN_SKEL_RIGHT_HIP      );
	drawLine( XN_SKEL_LEFT_HIP      , XN_SKEL_RIGHT_HIP      );
	drawLine( XN_SKEL_LEFT_HIP      , XN_SKEL_LEFT_KNEE      );
	drawLine( XN_SKEL_RIGHT_HIP     , XN_SKEL_RIGHT_KNEE     );
	drawLine( XN_SKEL_LEFT_KNEE     , XN_SKEL_LEFT_FOOT      );
	drawLine( XN_SKEL_RIGHT_KNEE    , XN_SKEL_RIGHT_FOOT     );

	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawLine( XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd )
{
	JointPositions::iterator it;
	it = mJointPositions.find( jointBeg );
	if( it == mJointPositions.end())
		return;

	Vec3f posBeg = it->second;

	it = mJointPositions.find( jointEnd );
	if( it == mJointPositions.end())
		return;

	Vec3f posEnd = it->second;

	RectMapping mapping( mUserManager->mOutputRect, mUserManager->mSourceBounds );
	gl::drawLine( posBeg, posEnd );
}

UserManager::UserManager()
: mJointColor( ColorA::hexA( 0x50ffffff ))
{
	mJoints.push_back( XN_SKEL_LEFT_HAND      );
	mJoints.push_back( XN_SKEL_LEFT_SHOULDER  );
	mJoints.push_back( XN_SKEL_HEAD           );
	mJoints.push_back( XN_SKEL_RIGHT_HAND     );
	mJoints.push_back( XN_SKEL_RIGHT_SHOULDER );
	mJoints.push_back( XN_SKEL_TORSO          );
	mJoints.push_back( XN_SKEL_LEFT_KNEE      );
	mJoints.push_back( XN_SKEL_RIGHT_KNEE     );
	mJoints.push_back( XN_SKEL_LEFT_FOOT      );
	mJoints.push_back( XN_SKEL_RIGHT_FOOT     );
	mJoints.push_back( XN_SKEL_NECK           );  // will not be visible only for body line
	mJoints.push_back( XN_SKEL_LEFT_HIP       );  // will not be visible only for body line
	mJoints.push_back( XN_SKEL_RIGHT_HIP      );  // will not be visible only for body line

	mJointRef = XN_SKEL_TORSO;
}

void UserManager::setup( const fs::path &path )
{
	mThread = thread( bind( &UserManager::openKinect, this, path ) );

	mParams = mndl::params::PInterfaceGl( "Kinect", Vec2i( 250, 350 ), Vec2i( 250, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addText("Tracking");
	mKinectProgress = "Connecting...\0\0\0\0\0\0\0\0\0";
	mParams.addParam( "Kinect", &mKinectProgress, "", true );
	mParams.addPersistentParam( "Skeleton smoothing" , &mSkeletonSmoothing, 0.9, "min=0 max=1 step=.05");
	mParams.addPersistentParam( "Joint show"         , &mJointShow, true  );
	mParams.addPersistentParam( "Line show"          , &mLineShow , true  );
	mParams.addPersistentParam( "Mirror"             , &mVideoMirrored, false );
//	mParams.addPersistentParam( "Video show"         , &mVideoShow, false );
	mVideoShow = false;
	mParams.addPersistentParam( "Joint size"         , &mJointSize, 20.0, "min=0 max=50 step=.5" );
	mParams.addPersistentParam( "Joint Color"        , &mJointColor, ColorA::hexA( 0x50c81e1e ) );

	mParams.addSeparator();

	using namespace boost::assign;
	vector< string > jointNames;
	jointNames += "Left hand", "Left shoulder", "Head", "Right hand", "Right shoulder",
		"Torso", "Left knee", "Right knee", "Left foot", "Right foot";
	for( size_t i = 0; i < jointNames.size(); i++ )
	{
		mParams.addPersistentParam( jointNames[ i ] + " active", &mRibbonActive[ i ], true );
	}

	mParams.setOptions( "", "refresh=.3" );

	mSourceBounds = app::getWindowBounds();
	setBounds( mSourceBounds );
}

void UserManager::openKinect( const fs::path &path )
{
	try
	{
		mndl::ni::OpenNI kinect;
		if ( path.empty() )
			kinect = OpenNI( OpenNI::Device() );
		else
			kinect = OpenNI( path );
		{
			boost::lock_guard< boost::mutex > lock( mMutex );
			mNI = kinect;
		}
	}
	catch ( ... )
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

	{
		boost::lock_guard< boost::mutex > lock( mMutex );
		mNI.setDepthAligned();
		mNI.start();
		mNIUserTracker = mNI.getUserTracker();
		mNIUserTracker.addListener( this );
	}
}

void UserManager::update()
{
	{
		boost::lock_guard< boost::mutex > lock( mMutex );
		if ( !mNI )
			return;
	}

	mNIUserTracker.setSmoothing( mSkeletonSmoothing );
	if ( mNI.isMirrored() != mVideoMirrored )
		mNI.setMirrored( mVideoMirrored );

	vector< unsigned > users = mNIUserTracker.getUsers();
	for( vector< unsigned >::const_iterator it = users.begin(); it != users.end(); ++it )
	{
		unsigned userId = *it;
		UserRef user = findUser( userId );
		if( ! user )
			continue;

		user->clearPoints();
		for( Joints::const_iterator it = mJoints.begin(); it != mJoints.end(); ++it )
		{
			XnSkeletonJoint jointId = *it;
			float conf = 0;
			Vec3f jointPos = mNIUserTracker.getJoint3d( userId, jointId, &conf );

			if( conf > .9 )
			{
				user->update( jointId, jointPos );
			}
		}
	}
}

void UserManager::draw( const Vec3f& cameraDir )
{
	drawRibbon( cameraDir );
	drawBody();
}

void UserManager::drawRibbon( const ci::Vec3f& cameraDir )
{
	gl::enableAlphaBlending();

	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->draw( cameraDir );
	}

	gl::disableAlphaBlending();
}

void UserManager::drawBody()
{
	gl::enableAlphaBlending();

	if( mNI && mNI.checkNewVideoFrame())
	{
		mNITexture = gl::Texture( mNI.getVideoImage() );
	}

	if( mNITexture && mVideoShow )
	{
		gl::color( Color::white() );
		gl::draw( mNITexture, mSourceBounds );
	}

	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->drawBody();
	}

	gl::disableAlphaBlending();
}

void UserManager::setBounds( const Rectf &rect )
{
	mOutputRect = rect;

	Rectf kRect( 0, 0, 640, 480 ); // kinect image rect
	Rectf dRect = kRect.getCenteredFit( mOutputRect, true );
	if( mOutputRect.getAspectRatio() > dRect.getAspectRatio() )
		dRect.scaleCentered( mOutputRect.getWidth() / dRect.getWidth() );
	else
		dRect.scaleCentered( mOutputRect.getHeight() / dRect.getHeight() );

	mOutputMapping = RectMapping( kRect, dRect, true );
}

void UserManager::clearRibbons()
{
	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->clearRibbons();
	}
}

void UserManager::createUser( unsigned userId )
{
	if( findUser( userId ))
		return;

	mUsers[ userId ] = UserRef( new User( this ));

	for( Joints::const_iterator it = mJoints.begin(); it != mJoints.end(); ++it )
	{
		XnSkeletonJoint jointId = *it;

		if( jointId == XN_SKEL_NECK
		 || jointId == XN_SKEL_LEFT_HIP
		 || jointId == XN_SKEL_RIGHT_HIP )
			continue;

		mUsers[ userId ]->addRibbon( jointId );
	}
}

void UserManager::destroyUser( unsigned userId )
{
	if( ! findUser( userId ))
		return;

	mUsers.erase( userId );
}

UserManager::UserRef UserManager::findUser( unsigned userId )
{
	Users::iterator it = mUsers.find( userId );
	if( it == mUsers.end())
		return UserRef();

	return it->second;
}

bool UserManager::getRibbonActive( XnSkeletonJoint jointId )
{
	switch( jointId )
	{
	case XN_SKEL_LEFT_HAND      : return mRibbonActive[ LEFT_HAND      ];
	case XN_SKEL_LEFT_SHOULDER  : return mRibbonActive[ LEFT_SHOULDER  ];
	case XN_SKEL_HEAD           : return mRibbonActive[ HEAD           ];
	case XN_SKEL_RIGHT_HAND     : return mRibbonActive[ RIGHT_HAND     ];
	case XN_SKEL_RIGHT_SHOULDER : return mRibbonActive[ RIGHT_SHOULDER ];
	case XN_SKEL_TORSO          : return mRibbonActive[ TORSO          ];
	case XN_SKEL_LEFT_KNEE      : return mRibbonActive[ LEFT_KNEE      ];
	case XN_SKEL_RIGHT_KNEE     : return mRibbonActive[ RIGHT_KNEE     ];
	case XN_SKEL_LEFT_FOOT      : return mRibbonActive[ LEFT_FOOT      ];
	case XN_SKEL_RIGHT_FOOT     : return mRibbonActive[ RIGHT_FOOT     ];
	}

	return false;
}

void UserManager::newUser( UserTracker::UserEvent event )
{
	console() << "new user: " << event.id << endl;
}

void UserManager::calibrationBeg( UserTracker::UserEvent event )
{
	console() << "user calib beg: " << event.id << endl;
}

void UserManager::calibrationEnd( UserTracker::UserEvent event )
{
	console() << "app calib end: " << event.id << endl;
	createUser( event.id );
}

void UserManager::lostUser( UserTracker::UserEvent event )
{
	console() << "lost user: " << event.id << endl;
	destroyUser( event.id );
}

void UserManager::keyUp( KeyEvent event )
{
	switch( event.getCode())
	{
	case KeyEvent::KEY_0 :
	case KeyEvent::KEY_1 :
	case KeyEvent::KEY_2 :
	case KeyEvent::KEY_3 :
	case KeyEvent::KEY_4 :
	case KeyEvent::KEY_5 :
	case KeyEvent::KEY_6 :
	case KeyEvent::KEY_7 :
	case KeyEvent::KEY_8 :
	case KeyEvent::KEY_9 :
		{
			int pos = event.getCode() - KeyEvent::KEY_0;
			mRibbonActive[ pos ] = ! mRibbonActive[ pos ];
		}
		break;
	}
}

} // namespace cinder
