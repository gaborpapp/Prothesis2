#pragma once

#include <deque>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "mndlkit/params/PParams.h"

typedef std::shared_ptr< class Ribbon > RibbonRef;

class Ribbon
{
	public:
		static RibbonRef create() { return RibbonRef( new Ribbon() ); }

		void addPos( const ci::Vec3f &pos );
		void update();
		void draw( const ci::Vec3f &cameraDir );
		void clear();

		void kill() { mState = STATE_DYING; mTargetCloseCount = 0; }
		bool isAlive() { return ( mState != STATE_DEAD ); }

		static void setMaxLength( int length ) { sMaxLength = length; }
		static void setWidth( float width ) { sWidth = width; }
		static void setStiffness( float k ) { sK = k; }
		static void setDamping( float damping ) { sDamping = damping; }
		static void setMass( float mass ) { sMass = mass; }

	protected:
		Ribbon() : mState( STATE_ALIVE )
		{}

		ci::Vec3f mTarget;
		ci::Vec3f mPos;
		ci::Vec3f mVel;
		std::deque< ci::Vec3f > mLoc;

		int mState;
		int mTargetCloseCount; // count if the target is close to the current location if dying

		enum
		{
			STATE_ALIVE = 0,
			STATE_DYING,
			STATE_DEAD
		};

		static int sMaxLength;
		static float sWidth;
		static float sK;
		static float sDamping;
		static float sMass;
};

