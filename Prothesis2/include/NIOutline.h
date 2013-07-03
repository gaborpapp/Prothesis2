#pragma once

#include <vector>

#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Shape2d.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"

typedef std::shared_ptr< class NIOutline > NIOutlineRef;

class NIOutline
{
	public:
		static NIOutlineRef create() { return NIOutlineRef( new NIOutline() ); }

		void update();
		void draw();

		bool isEnabled() const { return mEnabled; }

		bool *getEnabledValueRef() { return &mEnabled; }
		float *getBlurValueRef() { return &mBlurAmt; }
		float *getErodeValueRef() { return &mErodeAmt; }
		float *getDilateValueRef() { return &mDilateAmt; }
		int *getThresholdValueRef() { return &mThres; }
		float *getOutlineWidthValueRef() { return &mOutlineWidth; }
		ci::ColorA *getOutlineColorValueRef() { return &mOutlineColor; }

		void resize( const ci::Vec2i &size ) { mSize = size; }

	private:
		NIOutline();

		bool mEnabled;

		ci::Shape2d mShape;
		std::vector< ci::gl::VboMesh > mVboMeshes;
		ci::gl::GlslProg mShader;

		float mBlurAmt;
		float mErodeAmt;
		float mDilateAmt;
		int mThres;
		ci::ColorA mOutlineColor;
		float mOutlineWidth;

		ci::Vec2i mSize;
};

