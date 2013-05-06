#pragma once

#include "cinder/gl/Texture.h"

#include "ciMsaFluidDrawerGl.h"
#include "ciMsaFluidSolver.h"
#include "CinderOpenCV.h"

#include "Effect.h"
#include "FluidParticles.h"

class SmokeEffect;
typedef std::shared_ptr< SmokeEffect > SmokeEffectRef;

class SmokeEffect : public Effect
{
	public:
		static SmokeEffectRef create() { return SmokeEffectRef( new SmokeEffect() ); }

		void setup();

		void instantiate();

		void update();
		void draw();

	private:
		SmokeEffect() : Effect( "Smoke" ) {}

		ci::gl::Texture mCaptureTexture;

		// optflow
		bool mFlip;
		bool mDrawFlow;
		bool mDrawFluid;
		bool mDrawCapture;
		float mCaptureAlpha;
		float mFlowMultiplier;

		cv::Mat mPrevFrame;
		cv::Mat mFlow;

		int mOptFlowWidth;
		int mOptFlowHeight;

		// fluid
		ciMsaFluidSolver mFluidSolver;
		ciMsaFluidDrawerGl mFluidDrawer;

		int mFluidWidth, mFluidHeight;
		float mFluidFadeSpeed;
		float mFluidDeltaT;
		float mFluidViscosity;
		bool mFluidVorticityConfinement;
		bool mFluidWrapX, mFluidWrapY;
		float mFluidVelocityMult;
		float mFluidColorMult;
		ci::Color mFluidColor;
		ci::Color mBackgroundColor;

		// particles
		FluidParticleManager mParticles;
		float mParticleAging;
		int mParticleMin;
		int mParticleMax;
		float mMaxVelocity;
		float mVelParticleMult;
		float mVelParticleMin;
		float mVelParticleMax;
		ci::Color mParticleColor;

		void addToFluid( const ci::Vec2f &pos, const ci::Vec2f &vel, bool addParticles = true, bool addForce = true, bool addColor = true );
};

