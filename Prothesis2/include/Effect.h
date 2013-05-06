#pragma once

#include <string>

#include "cinder/Area.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"

#include "mndlkit/params/PParams.h"

class Effect;
typedef std::shared_ptr< Effect > EffectRef;

class Effect {
	public:
		static EffectRef create( const std::string &name ) { return EffectRef( new Effect( name ) ); }

		const std::string & getName() { return mName; }

		virtual void setup() {};

		//! Called when the effect becomes active.
		virtual void instantiate() {}
		//! Called when the effect becomes inactive.
		virtual void deinstantiate() {}

		void resize( const ci::Vec2i &size ) { mSize = size; }
		int getWidth() const { return mSize.x; }
		int getHeight() const { return mSize.y; }
		ci::Vec2i getSize() const { return mSize; }
		ci::Area getBounds() const { return ci::Area( 0, 0, mSize.x, mSize.y ); }

		virtual void update() {};
		virtual void draw() {};

	protected:
		Effect( const std::string &name ) : mName( name ) {}

		mndl::params::PInterfaceGl mParams;
		std::string mName;
		ci::Vec2i mSize;
};

