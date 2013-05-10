#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class Feedback;
typedef std::shared_ptr< Feedback > FeedbackRef;

class Feedback
{
	public:
		static FeedbackRef create( int w, int h ) { return FeedbackRef( new Feedback( w, h ) ); }

		ci::gl::Texture process( const ci::gl::Texture &source );

		bool isEnabled() const { return mEnabled; }

	private:
		Feedback( int w, int h );

		bool mEnabled;

		float mSpeed;
		float mFeed;
		float mNoiseSpeed;
		float mNoiseScale;
		float mNoiseDisp;
		float mNoiseTwirl;

		struct ExpParam
		{
			float start;
			float offset;
			float addPerFrame;
			float addPerPixel;
			float amplitude;
		};
		ExpParam mExpParams[ 2 ];
		void randomizeParams( unsigned long seed = 0 );

		static const int32_t DISP_SIZE = 512;
		ci::gl::GlslProg mShader;

		ci::gl::Fbo mFbo;
};

