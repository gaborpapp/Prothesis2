#pragma once

#include <string>
#include <vector>

#include "cinder/gl/Texture.h"
#include "cinder/MayaCamUI.h"

#include "CiNI.h"
#include "Effect.h"

class JointSpriteEffect;
typedef std::shared_ptr< JointSpriteEffect > JointSpriteEffectRef;

class JointSpriteEffect: public Effect
{
	public:
		static JointSpriteEffectRef create() { return JointSpriteEffectRef( new JointSpriteEffect() ); }

		void setup();

		void update();

		void draw();
		void drawControl();

		void mouseDown( ci::app::MouseEvent event );
		void mouseDrag( ci::app::MouseEvent event );

	private:
		JointSpriteEffect() : Effect( "JointSprite" ) {}

		ci::MayaCamUI mMayaCam;

		float mJointDisappearThr;

		void loadSprites( const ci::fs::path &relativeDir );
		std::vector< ci::gl::Texture > mSprites;
		std::vector< std::string > mSpriteNames;

		struct JointSprite
		{
			JointSprite() : lastSeen( -1000. ) { }
			double lastSeen;
			ci::Vec3f lastPos;
			int spriteId;
		};
		std::vector< JointSprite > mJointSprites;
};

