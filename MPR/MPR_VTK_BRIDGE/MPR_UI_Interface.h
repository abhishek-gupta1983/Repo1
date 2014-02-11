// MPR_VTK_BRIDGE.h

#pragma once

using namespace System;
#include "MPR.h"

using namespace RTViewer;
using namespace ImageUtils;
namespace MPR_VTK_BRIDGE {

	public ref class MPR_UI_Interface
	{
		
	private: // members
		static MPR_UI_Interface^ m_handle;
		MPR* m_mpr;

	public: // methods
		static MPR_UI_Interface^ GetHandle();
		~MPR_UI_Interface(void);
		void InitMPR(String^ path);
		BitmapWrapper^ GetDisplayImage(int axis);
		int GetNumberOfImages(int axis);
		void Scroll(int axis, int delta);
		int GetCurrentImageIndex(int axis);
		double GetCurrentImagePosition(int axis);
		double GetCurrentImagePositionRelativeToOrigin(int axis);
		
	protected:
		MPR_UI_Interface(void);
	};
}
