#include "Stdafx.h"
#include "MPR_UI_Interface.h"
#include "Utility_Functions.h"

// meddiff includes
#include "rad_util.h"
#include "rad_logger.h"

// VTK icludes
#include "vtkImageData.h"
#include "vtkSmartPointer.h"

using namespace System;
using namespace System::IO;
using namespace MPR_VTK_BRIDGE;
using namespace ImageUtils;

#define __FILENAME__ "MPR_UI_Interface.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPR_UI_Interface"

MPR_UI_Interface::MPR_UI_Interface()
{

	if (File::Exists("C:\\Temp\\MPR_View.log"))
	{
		File::Delete("C:\\Temp\\MPR_View.log");
	}
	rad_setLogFileName("C:\\Temp\\MPR_View.log");
	rad_setLogLevel(255);
}

MPR_UI_Interface^ MPR_UI_Interface::GetHandle()
{
	if (m_handle == nullptr)
	{
		m_handle = gcnew MPR_UI_Interface();
	}
	return m_handle;
}
MPR_UI_Interface::~MPR_UI_Interface(void)
{
}

void MPR_UI_Interface::InitMPR(String^ path)
{
	this->m_mpr = new MPR();

	// find all dcm files.
	array<String^>^ dirs = Directory::GetFiles(path, "*.dcm");
	vector<string> fileNames;
	for (int i = 0; i < dirs->Length; i++)
	{
		String^ _strFile = safe_cast<String^>(dirs[i]);
		const char* dicomFile = convert_to_const_charPtr(_strFile);
		fileNames.push_back(dicomFile);
	}
	this->m_mpr->initFromDir1(fileNames);
}

BitmapWrapper^ MPR_UI_Interface::GetDisplayImage(int axis)
{

	image displayImage = this->m_mpr->GetOutputImage((Axis)axis);
	
	BitmapWrapper^ bmp = gcnew BitmapWrapper(displayImage.data, displayImage.width, displayImage.height, "MONOCHROME");
	return bmp;
}

int MPR_UI_Interface::GetNumberOfImages(int axis)
{
	return this->m_mpr->GetNumberOfImages((Axis)axis);
}
void MPR_UI_Interface::Scroll(int axis, int delta)
{
	this->m_mpr->Scroll((Axis)axis, delta);
}

int MPR_UI_Interface::GetCurrentImageIndex(int axis)
{
	return this->m_mpr->GetCurrentImageIndex((Axis)axis);
}

double MPR_UI_Interface::GetCurrentImagePosition(int axis)
{
	return this->m_mpr->GetCurrentImagePosition((Axis)axis);
}