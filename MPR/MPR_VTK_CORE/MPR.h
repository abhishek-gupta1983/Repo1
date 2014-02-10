#pragma once
#include <iostream>
#include <vector>
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "enums.h"
#include "rad_util.h"
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
		void initFromDir1(vector<string> dicomFiles);
		void initFromImage(vtkSmartPointer<vtkImageData> image);
		image GetOutputImage(Axis axis);
		void Scroll(Axis axis, int delta);
		int GetNumberOfImages(Axis axis);
		int GetCurrentImageIndex(Axis axis);
		double GetCurrentImagePosition(Axis axis);
	private:
		MPRData* d;
	};
}