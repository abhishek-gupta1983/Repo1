
#include "MPRSlicer.h"

// VTK includes
#include "vtkImageReslice.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"

#include "vtkJPEGWriter.h"
using namespace RTViewer;

MPRSlicer::MPRSlicer(Axis axis)
{
	this->m_axis = axis;
	this->m_position = 0;
	this->m_inputImage = NULL;

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

	this->m_voilutFilter = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	this->m_voilutFilter->SetOutputFormatToLuminance();
	this->m_voilutFilter->SetWindow(400);
	this->m_voilutFilter->SetLevel(40);


}
void MPRSlicer::SetReslicePosition(double point[3])
{
	this->m_reslice->GetResliceAxes()->SetElement(0,3,point[0]);
	this->m_reslice->GetResliceAxes()->SetElement(1,3,point[1]);
	this->m_reslice->GetResliceAxes()->SetElement(2,3,point[2]);
}

vtkSmartPointer<vtkImageData> MPRSlicer::GetOutputImage()
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
	

	this->m_voilutFilter->SetInputData(this->m_reslice->GetOutput());
	this->m_voilutFilter->UpdateWholeExtent();

	this->m_outputImage = this->m_voilutFilter->GetOutput();
	return this->m_outputImage;
}

void MPRSlicer::Scroll(int delta)
{
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
			idx = this->m_spacing[2]==0 ? 0 : this->m_position/this->m_spacing[2];
			break;
		case CoronalAxis:
			idx = this->m_spacing[0] == 0 ? 0 : this->m_position / this->m_spacing[0];
			break;
		case SagittalAxis:
			idx = this->m_spacing[1] == 0 ? 0 : this->m_position / this->m_spacing[1];
			break;
		default:
			break;
	}
	return idx;
}