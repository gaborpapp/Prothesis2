#pragma once

#include <deque>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"

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

	protected:
		Ribbon()
		: mMaxLength( 128 )
		, mWidth( 16.0f )
		{}

		const unsigned short mMaxLength;
		const float mWidth;

		std::deque< ci::Vec3f > mLoc;

		bool mActive;
};

