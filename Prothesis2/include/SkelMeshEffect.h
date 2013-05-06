#pragma once

#include <deque>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"

#include "Effect.h"

class SkelMeshEffect;
typedef std::shared_ptr< SkelMeshEffect > SkelMeshEffectRef;

class SkelMeshEffect: public Effect
{
	public:
		static SkelMeshEffectRef create() { return SkelMeshEffectRef( new SkelMeshEffect() ); }

		void setup();

		void update();

		void draw();
		void drawControl();

		void mouseDown( ci::app::MouseEvent event );
		void mouseDrag( ci::app::MouseEvent event );

		void shutdown();

	private:
		SkelMeshEffect() : Effect( "SkelMesh" ), mEdgeNum( 0 ) {}

		mndl::params::PInterfaceGl mEdgeParams;

		ci::MayaCamUI mMayaCam;

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

		ci::Color mTrailMaterialAmbient;
		ci::Color mTrailMaterialDiffuse;
		ci::Color mTrailMaterialSpecular;
		float mTrailMaterialShininess;

		int mTrailSize;
		std::deque< ci::gl::VboMeshRef > mMeshes;

		ci::gl::GlslProg mPhongShader;

		void addEdge( int edgeId );
		void rebuildEdgeParams();
		void removeEdgeFromParams( int edgeId );

		int mEdgeNum;
		const int MAX_EDGE_NUM = 128;

		struct Edge
		{
			Edge() : mJoint0( 0 ), mJoint1( 0 ) {}
			int mJoint0, mJoint1;
		};
		std::vector< Edge > mEdges;

		void loadConfig( const ci::fs::path &fname );
		void saveConfig();
		ci::fs::path mConfigFile;
};

