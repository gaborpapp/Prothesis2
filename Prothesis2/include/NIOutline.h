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

		bool isEnabled() const { return mOutlineEnabled || mMaskEnabled; }

		bool *getMaskEnabledValueRef() { return &mMaskEnabled; }
		bool *getMultipleMasksEnabledValueRef() { return &mMultipleMasksEnabled; }
		bool *getOutlineEnabledValueRef() { return &mOutlineEnabled; }
		bool *getFlipValueRef() { return &mFlip; }
		float *getBlurValueRef() { return &mBlurAmt; }
		float *getErodeValueRef() { return &mErodeAmt; }
		float *getDilateValueRef() { return &mDilateAmt; }
		int *getThresholdValueRef() { return &mThres; }
		float *getOutlineWidthValueRef() { return &mOutlineWidth; }
		ci::ColorA *getMaskColor0ValueRef() { return &mMaskColor0; }
		ci::ColorA *getMaskColor1ValueRef() { return &mMaskColor1; }
		ci::ColorA *getOutlineColorValueRef() { return &mOutlineColor; }

		void resize( const ci::Vec2i &size ) { mSize = size; }

		ci::Vec2i getSize() const { return mSize; }
		ci::Area getBounds() const { return ci::Area( 0, 0, mSize.x, mSize. y ); }

	private:
		NIOutline();

		bool mMaskEnabled;
		bool mMultipleMasksEnabled;
		bool mOutlineEnabled;

		ci::Shape2d mShape;
		std::vector< ci::gl::VboMesh > mVboMeshes;
		ci::gl::GlslProg mShader;

		ci::gl::Texture mTexture0, mTexture1;

		bool mFlip;
		float mBlurAmt;
		float mErodeAmt;
		float mDilateAmt;
		int mThres;
		ci::ColorA mMaskColor0;
		ci::ColorA mMaskColor1;
		ci::ColorA mOutlineColor;
		float mOutlineWidth;

		ci::Vec2i mSize;
};

