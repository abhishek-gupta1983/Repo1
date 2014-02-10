
#include "MPRSlicer.h"
// meddiff includes

#include "rad_logger.h"
#include "streaming-image.h"
// VTK includes
#include "vtkImageReslice.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
using namespace RTViewer;

#define __FILENAME__ "MPRSlicer.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPRSlicer"

MPRSlicer::MPRSlicer(Axis axis)
{
	this->m_axis = axis;
	this->m_position = 0;
	this->m_inputImage = NULL;
	this->displayData = NULL;
	this->displayImage = ::born_image();
	this->displayImage.data = displayData;
}

MPRSlicer::~MPRSlicer(void)
{
	this->m_resliceMatrix->Delete();
	this->m_reslice->Delete();
}
void MPRSlicer::InitSlicer()
{
	// set up reslice.
	this->m_reslice = vtkSmartPointer<vtkImageReslice>::New();
	this->m_resliceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	this->m_resliceMatrix->Identity();

	switch (this->m_axis)
	{
		case AxialAxis:
			this->m_resliceMatrix->DeepCopy(axialElements);
			break;
		case SagittalAxis:
			this->m_resliceMatrix->DeepCopy(sagittalElements);
			break;
		case CoronalAxis:
			this->m_resliceMatrix->DeepCopy(coronalElements);
			break;
	}

	this->m_reslice->SetResliceAxes(this->m_resliceMatrix);
	this->m_reslice->InterpolateOff();
	this->m_reslice->SetOutputDimensionality(2);
	this->m_reslice->SetOutputSpacing(this->m_spacing);
	
	this->m_inputImage->GetOrigin(m_origin);
	switch (this->m_axis)
	{
		case CoronalAxis:
			this->m_position = m_origin[1];
			break;
		case AxialAxis:
			this->m_position = m_origin[2];
			break;
		case SagittalAxis:
			this->m_position = m_origin[0];
			break;
		default:
			break;
	}
	this->SetReslicePosition(m_origin);
}
void MPRSlicer::SetReslicePosition(double point[3])
{
	this->m_reslice->GetResliceAxes()->SetElement(0,3,point[0]);
	this->m_reslice->GetResliceAxes()->SetElement(1,3,point[1]);
	this->m_reslice->GetResliceAxes()->SetElement(2,3,point[2]);
}

image MPRSlicer::GetOutputImage()
{
	switch(this->m_axis)
	{
		case AxialAxis:
			{
				m_resliceMatrix->SetElement(0, 3, 0); 
				m_resliceMatrix->SetElement(1, 3, 0); 
				m_resliceMatrix->SetElement(2, 3, m_position); 
			}
			break;

		case CoronalAxis:
			{
				m_resliceMatrix->SetElement(0, 3, 0); 
				m_resliceMatrix->SetElement(1, 3, m_position); 
				m_resliceMatrix->SetElement(2, 3, 0); 
			}
			break;

		case SagittalAxis:
			{
				m_resliceMatrix->SetElement(0, 3, m_position); 
				m_resliceMatrix->SetElement(1, 3, 0); 
				m_resliceMatrix->SetElement(2, 3, 0); 
			}
			break;
	}
	this->m_reslice->SetResliceAxes(m_resliceMatrix);
	this->m_reslice->SetInputData(this->m_inputImage);
	this->m_reslice->SetOutputDimensionality(2); 
	this->m_reslice->SetInterpolationModeToCubic();
	this->m_reslice->Update();

	this->m_outputImage = this->m_reslice->GetOutput();

	if (this->m_outputImage != NULL)
	{
		int outDim[3] = { 0, 0, 0 };
		this->m_outputImage->GetDimensions(outDim);
		// Catch hold of arrays
		vtkDataArray* inScalars = this->m_outputImage->GetPointData()->GetScalars();

		image in_dcm = ::born_image();
		// Perform Window Level and Window width computations
		in_dcm.width = outDim[0];
		in_dcm.height = outDim[1];
		in_dcm.size = in_dcm.width*in_dcm.height;
		switch (inScalars->GetDataType())
		{
			case VTK_UNSIGNED_INT:
				in_dcm.type = TYPE_U32Data;
				break;
			case VTK_UNSIGNED_CHAR:
				in_dcm.type = TYPE_U8Data;
				break;
			case VTK_SHORT:
				in_dcm.type = TYPE_S16Data;
				break;
			case VTK_UNSIGNED_SHORT:
				in_dcm.type = TYPE_U16Data;
				break;
		}
		in_dcm.data = inScalars->GetVoidPointer(0);

		displayImage.width = in_dcm.width;
		displayImage.height = in_dcm.height;
		displayImage.size = in_dcm.size;
		displayImage.type = TYPE_U8Data;
		if (this->displayData != NULL)
		{
			rad_free_memory(this->displayData);
			this->displayData = NULL;
		}
		this->displayData = rad_get_memory(displayImage.height*displayImage.width*rad_sizeof(displayImage.type));
		displayImage.data = this->displayData;

		::voi_lut_transform_image_fast(
			displayImage,
			in_dcm,
			400,
			40,
			0, 255,
			1.0,
			-1024);

	}

	return displayImage;
}

void MPRSlicer::Scroll(int delta)
{
	RAD_LOG_INFO("Scrolling delta:" << delta);
	RAD_LOG_INFO("Old position:" << this->m_position);
	switch(this->m_axis)
	{
	case AxialAxis:
		this->m_position += delta*m_spacing[2];
		break;
	case CoronalAxis:
		this->m_position += delta*m_spacing[0];
		break;
	case SagittalAxis:
		this->m_position += delta*m_spacing[1];
	}
	RAD_LOG_INFO("New position:" << this->m_position);
	return;
}

int MPRSlicer::GetNumberOfImages()
{
	int num = 0;
	switch (this->m_axis)
	{
		case AxialAxis:
			num = this->m_dimension[2];
			break;
		case CoronalAxis:
			num = this->m_dimension[0];
			break;
		case SagittalAxis:
			num = this->m_dimension[1];
			break;
		default:
			break;
	}
	return num;
}

int MPRSlicer::GetSlicerPositionAsIndex()
{
	int idx = 0;
	switch (this->m_axis)
	{
		case AxialAxis:
			idx = this->m_spacing[2]==0 ? 0 : (this->m_position - this->m_origin[2])/this->m_spacing[2];
			break;
		case CoronalAxis:
			idx = this->m_spacing[0] == 0 ? 0 : (this->m_position - this->m_origin[0]) / this->m_spacing[0];
			break;
		case SagittalAxis:
			idx = this->m_spacing[1] == 0 ? 0 : (this->m_position - this->m_origin[1]) / this->m_spacing[1];
			break;
		default:
			break;
	}
	return abs(idx);
}

double MPRSlicer::GetSlicerPosition()
{
	return this->m_position;
}