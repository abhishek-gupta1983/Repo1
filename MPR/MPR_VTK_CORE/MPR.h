#pragma once
#include <iostream>
#include "vtkSmartPointer.h"
#include "enums.h"
class vtkImageData;
using namespace std;
namespace RTViewer
{
	struct MPRData;
	class MPR
	{
	public:
		MPR(void);
		~MPR(void);

		void initFromDir(string dirPath);
		void initFromImage(vtkSmartPointer<vtkImageData> image);
		vtkSmartPointer<vtkImageData> GetOutputImage(Axis axis);
		void Scroll(Axis axis, int delta);
		int GetNumberOfImages(Axis axis);
		int GetCurrentImageIndex(Axis axis);
	private:
		MPRData* d;
	};
}