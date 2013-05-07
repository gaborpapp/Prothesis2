#pragma once

#include <deque>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "mndlkit/params/PParams.h"

class Ribbon;
class RibbonManager;
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

		static void setMaxLength( int length ) { sMaxLength = length; }
		static void setWidth( float width ) { sWidth = width; }
		static void setMinDistance( float dist ) { sMinDistance = dist; }

	protected:
		Ribbon();

		std::deque< ci::Vec3f > mLoc;

		bool mActive;
		static int sMaxLength;
		static float sWidth;
		static float sMinDistance;
};

