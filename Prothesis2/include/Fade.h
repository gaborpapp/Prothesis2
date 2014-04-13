#pragma once

#include "cinder/Cinder.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

typedef std::shared_ptr< class Fade > FadeRef;

class Fade
{
	public:
		static FadeRef create( int w, int h ) { return FadeRef( new Fade( w, h ) ); }

		ci::gl::Texture process( const ci::gl::Texture &source );

		bool isEnabled() const { return mEnabled; }

	private:
		Fade( int w, int h );

		bool mEnabled;

		ci::Anim< float > mFade;
		float mFadeDuration;
		ci::Color mFadeColor;

		ci::gl::Fbo mFbo;
		ci::gl::GlslProgRef mShader;
};

