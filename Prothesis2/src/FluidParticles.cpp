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

#include "cinder/CinderMath.h"
#include "cinder/app/app.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "FluidParticles.h"

using namespace ci;
using namespace std;

const float FluidParticle::sMomentum = 0.6f;
const float FluidParticle::sFluidForce = 0.9f;

FluidParticle::FluidParticle()
	: mLifeSpan( 0 )
{
}

FluidParticle::FluidParticle( const Vec2f &pos )
{
	mColor = Color::white();
	mPos = pos;
	mVel = Vec2f( 0, 0 );
	mSize = Rand::randFloat( 10, 20 );
	mLifeSpan = Rand::randFloat( 0.3f, 1 );
	mMass = Rand::randFloat( 0.1f, 1 );
}

void FluidParticle::update( double time, const ciMsaFluidSolver *solver, const Vec2f &windowSize, const Vec2f &invWindowSize, float *positions, float *colors )
{
	mVel = solver->getVelocityAtPos( mPos * invWindowSize ) * (mMass * sFluidForce ) * windowSize + mVel * sMomentum;

	if ( mVel.lengthSquared() < 10 )
	{
		mVel += Rand::randVec2f() * 3.;
	}

	mPos += mVel;

	mLifeSpan *= FluidParticleManager::getAging();
	if ( mLifeSpan < 0.01f )
		mLifeSpan = 0;

	Vec2f velLimited = mVel.limited( 10 );

	positions[0] = mPos.x - velLimited.x;
	positions[1] = mPos.y - velLimited.y;
	positions[2] = mPos.x;
	positions[3] = mPos.y;

	colors[0] = mColor.r;
	colors[1] = mColor.g;
	colors[2] = mColor.b;
	colors[3] = mLifeSpan;
	colors[4] = mColor.r;
	colors[5] = mColor.g;
	colors[6] = mColor.b;
	colors[7] = mLifeSpan;
}

float FluidParticleManager::sAging = 0.995f;

FluidParticleManager::FluidParticleManager()
	: mCurrent( 0 ),
	  mActive( 0 )
{
	setWindowSize( Vec2i( 1, 1 ) );
}

void FluidParticleManager::setWindowSize( Vec2i winSize )
{
	mWindowSize = winSize;
	mInvWindowSize = Vec2f( 1.0f / winSize.x, 1.0f / winSize.y );
}

void FluidParticleManager::update( double seconds )
{
	int j = 0;
	mActive = 0;
	for ( int i = 0; i < MAX_PARTICLES; i++ )
	{
		if ( mParticles[i].isAlive() )
		{
			mParticles[i].update( seconds, mSolver,
					mWindowSize, mInvWindowSize,
					&mPositions[j * 2],
					&mColors[j * 4]);
			j += 2;
			mActive++;
		}
	}
}

void FluidParticleManager::draw()
{
	gl::enableAdditiveBlending();
	gl::disable( GL_TEXTURE_2D );
	gl::enable( GL_LINE_SMOOTH );

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, mPositions );

	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_FLOAT, 0, mColors );

	glDrawArrays( GL_LINES, 0, mActive * 2 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	gl::disableAlphaBlending();
}

void FluidParticleManager::addParticle( const Vec2f &pos, int count /* = 1 */ )
{
	mParticles[ mCurrent ] = FluidParticle( pos );
	mParticles[ mCurrent ].setColor( mParticleColor );
	for (int i = count - 1; i > 0; i--)
	{
		mCurrent = (mCurrent + 1) & (MAX_PARTICLES - 1);
		mParticles[ mCurrent ] = FluidParticle( pos + Rand::randVec2f() * 10 );
		mParticles[ mCurrent ].setColor( mParticleColor );
	}
	mCurrent = (mCurrent + 1) & (MAX_PARTICLES - 1);
}

