#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

typedef std::shared_ptr< class Kaleidoscope > KaleidoscopeRef;

class Kaleidoscope
{
	public:
		static KaleidoscopeRef create( int w, int h ) { return KaleidoscopeRef( new Kaleidoscope( w, h ) ); }

		ci::gl::Texture process( const ci::gl::Texture &source );

		bool isEnabled() const { return mEnabled; }

	private:
		Kaleidoscope( int w, int h );

		bool mEnabled;

		ci::gl::GlslProg mShader;
		int mNumReflectionLines;
		float mRotation;
		ci::Vec2f mCenter;

		ci::gl::Fbo mFbo;
};

