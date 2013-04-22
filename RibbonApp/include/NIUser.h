#pragma once

#include <map>
#include "cinder/app/App.h"

#include "CiNI.h"
#include "RibbonManager.h"
#include "mndlkit/params/PParams.h"

namespace cinder {

class UserManager;

class User
{
typedef std::map< XnSkeletonJoint, ci::Vec3f > JointPositions;

public:
	User( UserManager *userManager );

	void update( XnSkeletonJoint jointId, ci::Vec3f pos );
	void draw( const ci::Vec3f& cameraDir );

	void clearPoints();
	void addRibbon( XnSkeletonJoint jointId );
	void clearRibbons();
	void drawBody  ();

private:
	void drawJoints();
	void drawLines ();
	void drawLine  ( XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd );

private:
	UserManager     *mUserManager;
	JointPositions   mJointPositions;
	RibbonManager    mRibbonManager;
	ci::Vec3f        mPosRef;
};

class UserManager : mndl::ni::UserTracker::Listener
{
typedef std::shared_ptr< User >                                  UserRef;
typedef std::map< unsigned, UserRef >                            Users;
typedef std::vector< XnSkeletonJoint >                           Joints;
public:
	UserManager();
	~UserManager();

	void setup( const ci::fs::path &path = "" );
	void update();
	void draw( const ci::Vec3f& cameraDir );
	void drawRibbon( const ci::Vec3f& cameraDir );
	void drawBody  ();

	void setRibbonLength( unsigned short ribbonLength );

	void setBounds( const Rectf &rect );
	void clearRibbons();

	void newUser       ( mndl::ni::UserTracker::UserEvent event );
	void lostUser      ( mndl::ni::UserTracker::UserEvent event );
	void calibrationBeg( mndl::ni::UserTracker::UserEvent event );
	void calibrationEnd( mndl::ni::UserTracker::UserEvent event );

	void keyUp( ci::app::KeyEvent event );

private:
	void    createUser ( unsigned userId );
	void    destroyUser( unsigned userId );
	UserRef findUser   ( unsigned userId );

	bool            getRibbonActive( XnSkeletonJoint jointId );

private:
	enum JointType
	{
		LEFT_HAND      = 0,
		LEFT_SHOULDER  = 1,
		HEAD           = 2,
		RIGHT_HAND     = 3,
		RIGHT_SHOULDER = 4,
		TORSO          = 5,
		LEFT_KNEE      = 6,
		RIGHT_KNEE     = 7,
		LEFT_FOOT      = 8,
		RIGHT_FOOT     = 9,
	};

	mndl::ni::OpenNI      mNI;
	mndl::ni::UserTracker mNIUserTracker;

	Joints  mJoints;
	XnSkeletonJoint mJointRef;

	ci::Rectf       mOutputRect;
	ci::RectMapping mOutputMapping;
	ci::Area        mSourceBounds;

	ci::gl::Texture     mNITexture;

	// params0
	mndl::params::PInterfaceGl mParams;
	std::string                mKinectProgress;
	float                      mSkeletonSmoothing;
	bool                       mJointShow;
	bool                       mLineShow;
	bool                       mVideoShow;
	bool                       mVideoMirrored;
	float                      mJointSize;
	ci::ColorA                 mJointColor;

	int                        mRibbonSelect[10];
	bool                       mRibbonActive[10];

	std::shared_ptr< std::thread > mThread;
	std::mutex                 mMutex;
	void                       openKinect( const ci::fs::path &path );

	Users                      mUsers;

	friend class User;
};

} // namespace cinder
