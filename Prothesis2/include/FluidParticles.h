/*
 Copyright (C) 2013 Gabor Papp, Botond Gabor Barna

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "cinder/Vector.h"
#include "cinder/Color.h"

#include "ciMsaFluidSolver.h"

class FluidParticle
{
	public:
		FluidParticle();
		FluidParticle( const ci::Vec2f &pos );

		void update( double time, const ciMsaFluidSolver *solver, const ci::Vec2f &windowSize, const ci::Vec2f &invWindowSize, float *positions, float *colors );
		bool isAlive() { return mLifeSpan > 0; }

		void setColor( const ci::Color &color ) { mColor = color; }

	private:
		ci::Vec2f mPos;
		ci::Vec2f mVel;
		ci::ColorA mColor;

		float mSize;
		float mLifeSpan;
		float mMass;

		static const float sMomentum;
		static const float sFluidForce;
};

class FluidParticleManager
{
	public:
		FluidParticleManager();

		void setWindowSize( ci::Vec2i winSize );
		void setFluidSolver( const ciMsaFluidSolver *aSolver ) { mSolver = aSolver; }

		void update( double seconds );
		void draw();

		void addParticle( const ci::Vec2f &pos, int count = 1 );

		static float getAging() { return sAging; }
		static void setAging( float a ) { sAging = a; }

		void setColor( const ci::Color &color ) { mParticleColor = color; }

	private:
		ci::Vec2i mWindowSize;
		ci::Vec2f mInvWindowSize;

		const ciMsaFluidSolver *mSolver;

		static float sAging;
		ci::Color mParticleColor;

#define MAX_PARTICLES 16384 // pow 2!
		int mCurrent;
		int mActive;

		float mPositions[ MAX_PARTICLES * 2 * 2 ];
		float mColors[ MAX_PARTICLES * 4 * 2 ];
		FluidParticle mParticles[ MAX_PARTICLES ];
};


