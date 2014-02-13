#pragma once
#include <iostream>

#include "rad_util.h"
#include "enums.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
class vtkMatrix4x4;
class vtkImageReslice;
using namespace std;
namespace RTViewer
{
	// forward declaration;
	class MPR;
	class MPRSlicer
	{
		protected:
			MPRSlicer(Axis axis);
			~MPRSlicer(void);
		public:
			// Set input image
			// @param: vtkImageData
			void SetInput(vtkSmartPointer<vtkImageData> input){
				this->m_inputImage = input;
				this->m_inputImage->GetSpacing(m_spacing);
				this->m_inputImage->GetDimensions(m_dimension);
			}
			void InitSlicer();
			void SetResliceMatrix(vtkSmartPointer<vtkMatrix4x4> matrix){this->m_resliceMatrix = matrix;}
			void SetReslicePosition(double point[3]);
			image GetOutputImage();
			void Scroll(int delta);
			int GetNumberOfImages();
			int GetSlicerPositionAsIndex();
			double GetSlicerPosition();
			string GetOrientationMarkers_L(){ return this->m_orientatationMarkers_L; }
			string GetOrientationMarkers_R(){ return this->m_orientatationMarkers_R; }
			string GetOrientationMarkers_T(){ return this->m_orientatationMarkers_T; }
			string GetOrientationMarkers_B(){ return this->m_orientatationMarkers_B; }

	protected: // methods
		void ComputeOrientationMarkers();

		private:
			Axis m_axis; // slicer axis
			vtkSmartPointer<vtkImageReslice> m_reslice; // actual slicer
			vtkSmartPointer<vtkMatrix4x4> m_resliceMatrix; // matrix used for slicer orientation & position
			vtkSmartPointer<vtkImageData> m_inputImage; // input vtkImageData; i.e. image cuboid
			vtkSmartPointer<vtkImageData> m_outputImage; // output sliced image.
			string m_orientatationMarkers_L;
			string m_orientatationMarkers_R;
			string m_orientatationMarkers_T;
			string m_orientatationMarkers_B;
			double m_position;
			double m_spacing[3];
			int m_dimension[3];
			double m_origin[3];
			void* displayData;
			image displayImage;

		friend class MPR;
	};
}