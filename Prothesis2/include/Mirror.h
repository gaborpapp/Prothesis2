#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

typedef std::shared_ptr< class Mirror > MirrorRef;

class Mirror
{
	public:
		static MirrorRef create( int w, int h ) { return MirrorRef( new Mirror( w, h ) ); }

		ci::gl::Texture process( const ci::gl::Texture &source );

		bool isEnabled() const { return mEnabled; }

	private:
		Mirror( int w, int h );

		bool mEnabled;

		bool mFlipVertical;
		bool mFlipHorizontal;

		ci::gl::Fbo mFbo;
};

