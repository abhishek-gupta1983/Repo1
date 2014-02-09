#include "Stdafx.h"
#include "MPR_UI_Interface.h"
#include "Utility_Functions.h"

#include "vtkImageData.h"
#include "vtkSmartPointer.h"

using namespace System;
using namespace MPR_VTK_BRIDGE;
using namespace ImageUtils;

MPR_UI_Interface::MPR_UI_Interface()
{

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
	const char* _path = convert_to_const_charPtr(path);
	this->m_mpr->initFromDir(_path);
}

BitmapWrapper^ MPR_UI_Interface::GetDisplayImage(int axis)
{

	vtkSmartPointer<vtkImageData> image = this->m_mpr->GetOutputImage((Axis)axis);
	int dim[3] = { 0, 0, 0 };
	image->GetDimensions(dim);
	BitmapWrapper^ bmp = gcnew BitmapWrapper(image->GetScalarPointer(), dim[0], dim[1], "MONOCHROME");
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