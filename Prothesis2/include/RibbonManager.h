#pragma once

#include <map>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/Vector.h"

#include "Ribbon.h"

class RibbonManager
{
typedef std::map< int, RibbonRef > Ribbons;

public:
	void update();
	void draw( const ci::Vec3f& cameraDir );

	void clear();

	RibbonRef createRibbon( int id );
	void detachRibbon( int id );
	void destroyRibbon( int id );
	RibbonRef findRibbon( int id );

private:
	Ribbons mRibbons;
	std::vector< RibbonRef > mDyingRibbons;
};

