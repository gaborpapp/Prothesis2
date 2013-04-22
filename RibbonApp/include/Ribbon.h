#pragma once

#include <deque>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "mndlkit/params/PParams.h"

class Ribbon;
typedef std::shared_ptr< Ribbon > RibbonRef;

class Ribbon
{
	public:
		static RibbonRef create() { return RibbonRef( new Ribbon() ); }

		void update( const ci::Vec3f &pos );
		void draw( const ci::Vec3f &cameraDir );
		void clear();

		void setActive( bool active );
		bool getActive() const;

		static void setup();

	protected:
		static mndl::params::PInterfaceGl mParams;
		static int mMaxLength;
		static float mWidth;
		static float mMinDistance;

		std::deque< ci::Vec3f > mLoc;

		bool mActive;
};

