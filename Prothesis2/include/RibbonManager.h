#pragma once

#include <map>

#include "cinder/app/App.h"
#include "cinder/Vector.h"

#include "Ribbon.h"

class RibbonManager
{
typedef std::map< int, RibbonRef > Ribbons;

public:
	void setup();

	void update( int id, ci::Vec3f pos );
	void draw( const ci::Vec3f& cameraDir );
	void drawControl();

	void setActive( int id, bool active );
	void clear();

	int  createRibbon ( int id );
	void destroyRibbon( int id );
	RibbonRef findRibbon( int id );

	int   getMaxLength() const;
	float getWidth() const;
	float getMinDistance() const;

private:
	Ribbons                    mRibbons;

	mndl::params::PInterfaceGl mParams;
	int                        mMaxLength;
	float                      mWidth;
	float                      mMinDistance;
};

