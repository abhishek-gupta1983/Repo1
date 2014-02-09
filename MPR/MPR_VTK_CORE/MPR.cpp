
#include "MPR.h"
#include "MPRSlicer.h"

// VTK includes
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkDicomImageReader.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

using namespace RTViewer;

namespace RTViewer
{
	struct MPRData
	{
	private:
		vtkSmartPointer<vtkImageData> m_inputImage;

	public:
		MPRSlicer* m_slicers[3];
		
	private:
		
	public:
		MPRSlicer* GetSlicer(Axis axis)
		{
			return m_slicers[axis];
		}
		void SetInput(vtkSmartPointer<vtkImageData> image) { this->m_inputImage = image; }
		vtkSmartPointer<vtkImageData> GetInput() { return this->m_inputImage; }
		
	};
}
MPR::MPR(void)
{
	
	d = new MPRData();

}

MPR::~MPR(void)
{
	for (int i = 0; i<3; i++)
	{
		delete d->m_slicers[i];
	}
	delete d;
}

void MPR::initFromDir(string dirPath)
{
	vtkSmartPointer<vtkDICOMImageReader> dicomReader  = vtkSmartPointer<vtkDICOMImageReader>::New();
	dicomReader->SetDirectoryName(dirPath.c_str());
	dicomReader->Update();
	this->initFromImage(dicomReader->GetOutput());
}

void MPR::initFromImage(vtkSmartPointer<vtkImageData> image)
{
	if( d->GetInput()!=NULL )
	{
		d->GetInput()->Delete();
		d->SetInput(NULL);
	}
	d->SetInput(image);
	for(int i=0;i<3;i++)
	{
		MPRSlicer* slicer = new MPRSlicer((Axis)i);
		slicer->SetInput(image);
		slicer->InitSlicer();
		d->m_slicers[i] = slicer;
	}

	double center[3]={0,0,0};
	d->GetInput()->GetCenter(center);
}

vtkSmartPointer<vtkImageData> MPR::GetOutputImage(Axis axis)
{
	for(int i=0;i<3;i++)
	{
		if(i==axis)
		{
			return d->m_slicers[i]->GetOutputImage();
		}
	}
	return NULL;
}

void MPR::Scroll(Axis axis, int delta)
{
	for(int i=0; i<3; i++)
	{
		if(i == axis)
		{
			d->m_slicers[i]->Scroll(delta);
		}
	}
}