#pragma once

#include "cinder/app/App.h"

#include "CiNI.h"

class GlobalData
{
	private:
		//! Singleton implementation
		GlobalData() {}
		~GlobalData() {};

	public:
		static GlobalData & get() { static GlobalData data; return data; }

		ci::app::WindowRef mOutputWindow;
		ci::app::WindowRef mControlWindow;

		mndl::ni::OpenNI mNI;
		mndl::ni::UserTracker mNIUserTracker;
};

