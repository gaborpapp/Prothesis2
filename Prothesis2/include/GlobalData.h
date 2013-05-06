#pragma once

#include "cinder/app/App.h"

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
};

