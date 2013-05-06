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
		static RibbonRef create( RibbonManager* ribbonManager ) { return RibbonRef( new Ribbon( ribbonManager ) ); }

		Ribbon( RibbonManager* ribbonManager );

		void update( const ci::Vec3f &pos );
		void draw( const ci::Vec3f &cameraDir );
		void clear();

		void setActive( bool active );
		bool getActive() const;

	protected:
		RibbonManager* mRibbonManager;

		std::deque< ci::Vec3f > mLoc;

		bool mActive;
};

