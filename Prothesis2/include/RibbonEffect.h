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

		void shutdown();

	private:
		RibbonEffect() : Effect( "Ribbon" ) {}

		mndl::params::PInterfaceGl mEdgeParams;

		ci::MayaCamUI mMayaCam;

		RibbonManager mRibbonManager;

#define XN_SKEL_MIN XN_SKEL_HEAD // NOTE: starts at 1
#define XN_SKEL_MAX XN_SKEL_RIGHT_FOOT
#define XN_SKEL_NUM ( XN_SKEL_RIGHT_FOOT + 1 )

		struct Position
		{
			Position()
			{
				for ( int i = 0; i < XN_SKEL_NUM; i++ )
				{
					mConf[ i ] = 0.f;
				}
			}
			ci::Vec3f mJoints[ XN_SKEL_NUM ];
			float mConf[ XN_SKEL_NUM ];
		};

		std::shared_ptr< Position > mPositionRef;
		std::shared_ptr< Position > mPrevPositionRef;

		void updateSkeleton();

		ci::Vec3f mLightDirection;
		ci::Color mLightAmbient;
		ci::Color mLightDiffuse;
		ci::Color mLightSpecular;

		ci::Color mMaterialAmbient;
		ci::Color mMaterialDiffuse;
		ci::Color mMaterialSpecular;
		float mMaterialShininess;
};

