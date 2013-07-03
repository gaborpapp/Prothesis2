#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "CinderOpenCV.h"
#include "opencv2/imgproc/imgproc.hpp"

#include "NIOutline.h"
#include "GlobalData.h"
#include "Resources.h"

using namespace ci;
using namespace std;

NIOutline::NIOutline()
{
	mShader = gl::GlslProg( app::loadResource( RES_LINE_VERT ),
							app::loadResource( RES_LINE_FRAG ),
							app::loadResource( RES_LINE_GEOM ),
							GL_LINES_ADJACENCY_EXT, GL_TRIANGLE_STRIP, 7 );
}

void NIOutline::update()
{
	if ( !mEnabled )
		return;

	GlobalData &gd = GlobalData::get();
	if ( !gd.mNI )
		return;

	if ( gd.mNI && gd.mNI.isCapturing() && gd.mNI.checkNewVideoFrame() )
	{
		Surface8u maskSurface = gd.mNIUserTracker.getUserMask();

		cv::Mat cvMask, cvMaskFiltered;
		cvMask = toOcv( Channel8u( maskSurface ) );
		cv::blur( cvMask, cvMaskFiltered, cv::Size( mBlurAmt, mBlurAmt ) );

		cv::Mat dilateElm = cv::getStructuringElement( cv::MORPH_RECT,
				cv::Size( mDilateAmt, mDilateAmt ) );
		cv::Mat erodeElm = cv::getStructuringElement( cv::MORPH_RECT,
				cv::Size( mErodeAmt, mErodeAmt ) );
		cv::erode( cvMaskFiltered, cvMaskFiltered, erodeElm, cv::Point( -1, -1 ), 1 );
		cv::dilate( cvMaskFiltered, cvMaskFiltered, dilateElm, cv::Point( -1, -1 ), 3 );
		cv::erode( cvMaskFiltered, cvMaskFiltered, erodeElm, cv::Point( -1, -1 ), 1 );
		cv::blur( cvMaskFiltered, cvMaskFiltered, cv::Size( mBlurAmt, mBlurAmt ) );

		cv::threshold( cvMaskFiltered, cvMaskFiltered, mThres, 255, CV_THRESH_BINARY);

		vector< vector< cv::Point > > contours;
		cv::findContours( cvMaskFiltered, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );

		mShape.clear();
		for ( vector< vector< cv::Point > >::const_iterator it = contours.begin();
				it != contours.end(); ++it )
		{
			vector< cv::Point >::const_iterator pit = it->begin();

			if ( it->empty() )
				continue;

			mShape.moveTo( fromOcv( *pit ) );

			++pit;
			for ( ; pit != it->end(); ++pit )
			{
				mShape.lineTo( fromOcv( *pit ) );
			}
			mShape.close();
		}
	}

	// update vbo from shape points
	// based on the work of Paul Houx
	// https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader#23286000001297067
	mVboMeshes.clear();

	for ( size_t i = 0; i < mShape.getNumContours(); ++i )
	{
		const Path2d &path = mShape.getContour( i );
		const vector< Vec2f > &points = path.getPoints();

		if ( points.size() <= 1 )
			continue;

		// create a new vector that can contain 3D vertices
		vector< Vec3f > vertices;

		vertices.reserve( points.size() );

		// add all 2D points as 3D vertices
		vector< Vec2f >::const_iterator it;
		for ( it = points.begin() ; it != points.end(); ++it )
			vertices.push_back( Vec3f( *it ) );

		// now that we have a list of vertices, create the index buffer
		size_t n = vertices.size();

		vector< uint32_t > indices;
		indices.reserve( n * 4 );

		// line loop
		indices.push_back( n - 1 );
		indices.push_back( 0 );
		indices.push_back( 1 );
		indices.push_back( 2 );
		for ( size_t i = 1; i < vertices.size() - 2; ++i )
		{
			indices.push_back( i - 1 );
			indices.push_back( i );
			indices.push_back( i + 1 );
			indices.push_back( i + 2 );
		}
		indices.push_back( n - 3 );
		indices.push_back( n - 2 );
		indices.push_back( n - 1 );
		indices.push_back( 0 );

		indices.push_back( n - 2 );
		indices.push_back( n - 1 );
		indices.push_back( 0 );
		indices.push_back( 1 );

		// finally, create the mesh
		gl::VboMesh::Layout layout;
		layout.setStaticPositions();
		layout.setStaticIndices();
		gl::VboMesh vboMesh = gl::VboMesh( vertices.size(), indices.size(), layout, GL_LINES_ADJACENCY_EXT );
		vboMesh.bufferPositions( &(vertices.front()), vertices.size() );
		vboMesh.bufferIndices( indices );
		vboMesh.unbindBuffers();

		mVboMeshes.push_back( vboMesh );
	}
}

void NIOutline::draw()
{
	if ( !mEnabled )
		return;

	gl::setViewport( Area( 0, 0, mSize.x, mSize.y ) );
	gl::setMatricesWindow( mSize );

	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();

	gl::color( mOutlineColor );
	mShader.bind();
	mShader.uniform( "WIN_SCALE", Vec2f( mSize ) );
	mShader.uniform( "MITER_LIMIT", .75f );
	mShader.uniform( "THICKNESS", mOutlineWidth );

	for ( vector< gl::VboMesh >::const_iterator vit = mVboMeshes.begin();
			vit != mVboMeshes.end(); ++vit )
	{
		gl::draw( *vit );
	}

	mShader.unbind();
	gl::disableAlphaBlending();
}

