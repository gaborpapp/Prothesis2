#pragma once

#include <map>

#include "cinder/app/App.h"
#include "cinder/Vector.h"

#include "Ribbon.h"

class RibbonManager
{
typedef std::map< int, RibbonRef > Ribbons;

public:
	void update( int id, ci::Vec3f pos );
	void draw( const ci::Vec3f& cameraDir );

	void setActive( int id, bool active );
	void clear();

	RibbonRef createRibbon ( int id );
	void destroyRibbon( int id );
	RibbonRef findRibbon( int id );

private:
	Ribbons                    mRibbons;
};

