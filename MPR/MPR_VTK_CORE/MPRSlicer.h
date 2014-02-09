#pragma once
#include "enums.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
class vtkMatrix4x4;
class vtkImageReslice;
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
			vtkSmartPointer<vtkImageData> GetOutputImage();
			void Scroll(int delta);
		private:
			Axis m_axis; // slicer axis
			vtkSmartPointer<vtkImageReslice> m_reslice; // actual slicer
			vtkSmartPointer<vtkMatrix4x4> m_resliceMatrix; // matrix used for slicer orientation & position
			vtkSmartPointer<vtkImageData> m_inputImage; // input vtkImageData; i.e. image cuboid
			vtkSmartPointer<vtkImageData> m_outputImage; // output sliced image.
			vtkSmartPointer<vtkImageMapToWindowLevelColors> m_voilutFilter; // VOI LUT Filter from VTK.
			double m_position;
			double m_spacing[3];
			int m_dimension[3];

		friend class MPR;
	};
}