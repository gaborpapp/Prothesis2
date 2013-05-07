#pragma once

#include <deque>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"

#include "Effect.h"
#include "RibbonManager.h"

class RibbonEffect;
typedef std::shared_ptr< RibbonEffect > RibbonEffectRef;

class RibbonEffect: public Effect
{
	public:
		static RibbonEffectRef create() { return RibbonEffectRef( new RibbonEffect() ); }

		void setup();

		void update();

		void draw();
		void drawControl();

		void mouseDown( ci::app::MouseEvent event );
		void mouseDrag( ci::app::MouseEvent event );

	private:
		RibbonEffect() : Effect( "Ribbon" ) {}

		ci::MayaCamUI mMayaCam;

		RibbonManager mRibbonManager;

		ci::Vec3f mLightDirection;
		ci::Color mLightAmbient;
		ci::Color mLightDiffuse;
		ci::Color mLightSpecular;

		ci::Color mMaterialAmbient;
		ci::Color mMaterialDiffuse;
		ci::Color mMaterialSpecular;
		float mMaterialShininess;

		int mRibbonMaxLength;
		float mRibbonWidth;
		float mRibbonMinPointDistance;

#define XN_SKEL_MIN XN_SKEL_HEAD // NOTE: starts at 1
#define XN_SKEL_MAX XN_SKEL_RIGHT_FOOT
#define XN_SKEL_NUM ( XN_SKEL_RIGHT_FOOT + 1 )
		bool mRibbonActive[ XN_SKEL_NUM + XN_SKEL_HEAD ];
};

