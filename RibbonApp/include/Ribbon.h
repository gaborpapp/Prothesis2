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

		void draw();

	protected:
		Ribbon() {}

		const int mMaxLength = 128;

		std::deque< ci::Vec3f > mLoc;
};

