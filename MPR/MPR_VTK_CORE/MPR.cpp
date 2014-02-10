
#include "MPR.h"
#include "MPRSlicer.h"

// meddiff includes
#include "rad_util.h"
#include "rad_logger.h"
#include "RTDcmtkDicomInterface.h"
#include "rad_template_helper.h"

//dcmtk includes

// VTK includes
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkDicomImageReader.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include <vtkUnsignedIntArray.h>
#include <vtkSignedCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>

using namespace RTViewer;

#define __FILENAME__ "MPR.cpp"
#undef __MODULENAME__
#define __MODULENAME__ "MPR"

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

void MPR::initFromDir1(vector<string> dicomFiles)
{
	// load data from scratch.
	map<float, string> sortedDicomFiles;
	map<float, string>::iterator rit;
	
	for (int i = 0; i < dicomFiles.size(); i++)
	{
		RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(dicomFiles.at(i).c_str());
		if (!pDicom->IsFileLoadedSuccessfully())
		{
			delete pDicom;
			continue;
		}
		if (!((RTDcmtkDicomInterface*)pDicom)->checkIfRTObject())
		{
			string imagePosition = "";
			vector<string> temp;
			imagePosition = pDicom->Get_IMAGE_POSITION();
			tokenize(imagePosition, temp, "\\");
			sortedDicomFiles.insert(std::pair<float, std::string>(convert_to_double(temp[2].c_str()), dicomFiles.at(i)));
		}
		delete pDicom;
	}

	RadDataType dicomDataType = TYPE_NOT_SET;
	void* dicomData;
	double firstImagePosition[3];
	int dimensions[3];
	double spacing[3];
	int dicomDataIdx;
	int dicomSliceSize;
	int dicomDataSize;
	int j = 0;
	for (rit = sortedDicomFiles.begin(); rit != sortedDicomFiles.end(); ++rit)
	{
		
		const char* dicomFile = rit->second.c_str();
		RadRTDicomInterface* pDicom = new RTDcmtkDicomInterface(dicomFile);
		if (j == 0)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition, _imgPosition, "\\", true);
			for (int i = 0; i<_imgPosition.size(); i++)
			{
				firstImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
			}

			if (pDicom->Get_BITS_ALLOCATED() / 8 == 1)
			{
				if (!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "RGB") ||
					!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(), "PALETTE COLOR"))
					dicomDataType = TYPE_U32Data;
				else
					dicomDataType = TYPE_U8Data;
			}
			else
			{
				if (!pDicom->Get_PIXEL_REPRESENTATION())
					dicomDataType = TYPE_U16Data;
				else
					dicomDataType = TYPE_S16Data;
			}
			dimensions[0] = pDicom->Get_ROW();
			dimensions[1] = pDicom->Get_COLOUMN();
			dimensions[2] = dicomFiles.size();
			dicomDataSize = dimensions[0] * dimensions[1] * dimensions[2];
			dicomSliceSize = dimensions[0] * dimensions[1];

			string pixelSpacing = string(pDicom->Get_PIXEL_SPACING());
			vector<string> _pixelSpacing;
			tokenize(pixelSpacing, _pixelSpacing, "\\", true);

			spacing[0] = convert_to_double(_pixelSpacing.at(0).c_str());
			spacing[1] = convert_to_double(_pixelSpacing.at(1).c_str());
			spacing[2] = firstImagePosition[2];

			RAD_LOG_CRITICAL(dicomDataType);
			switch (dicomDataType)
			{
				case TYPE_U32Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U32Data));
					break;
				case TYPE_U8Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U8Data));
					break;
				case TYPE_U16Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U16Data));
					break;
				case TYPE_S16Data:
					dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_S16Data));
					break;
			}

			dicomDataIdx = dicomSliceSize;
		}
		if (j == 1)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition, _imgPosition, "\\", true);
			spacing[2] = convert_to_double(_imgPosition.at(2).c_str()) - spacing[2];
		}

		image pixelData = born_image();
		image overlayPixelData = born_image();
		pDicom->InflateSingleFrameDicomPixelData(&pixelData, &overlayPixelData);
		void* raw_dcm_data = pixelData.data;

		switch (dicomDataType)
		{
			case TYPE_U32Data: {
				U32DataType* dicomData2 = static_cast<U32DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U32DataType));
			} break;
			case TYPE_U8Data: {
				U8DataType* dicomData2 = static_cast<U8DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U8DataType));
			} break;
			case TYPE_U16Data: {
				U16DataType* dicomData2 = static_cast<U16DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(U16DataType));
			} break;
			case TYPE_S16Data: {
				S16DataType* dicomData2 = static_cast<S16DataType*>(dicomData);
				dicomData2 += dicomDataSize;
				memcpy(dicomData2 - dicomDataIdx, raw_dcm_data, dicomSliceSize*sizeof(S16DataType));
			} break;
		}
		dicomDataIdx += dicomSliceSize;

		if (pixelData.data)
		{
			rad_free_memory(pixelData.data);
		}
		if (overlayPixelData.data)
		{
			rad_free_memory(overlayPixelData.data);
		}

		delete pDicom;
		j++;
	}
	dicomDataIdx -= dicomSliceSize;
	vtkDataArray* scalars = 0;

	switch (dicomDataType)
	{
		case TYPE_U32Data:
			scalars = vtkUnsignedIntArray::New();
			((vtkUnsignedIntArray*)(scalars))->SetArray((unsigned int*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_U8Data:
			scalars = vtkUnsignedCharArray::New();
			((vtkUnsignedCharArray*)(scalars))->SetArray((unsigned char*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_U16Data:
			scalars = vtkUnsignedShortArray::New();
			((vtkUnsignedShortArray*)(scalars))->SetArray((unsigned short*)dicomData, dicomDataSize, 1);
			break;
		case TYPE_S16Data:
			scalars = vtkShortArray::New();
			((vtkShortArray*)(scalars))->SetArray((short*)dicomData, dicomDataSize, 1);
			break;
	}
	scalars->SetNumberOfComponents(1);

	vtkSmartPointer<vtkImageData> CTMPRCuboid = vtkSmartPointer<vtkImageData>::New();
	//vtkImageData* CTMPRCuboid = vtkImageData::New();
	CTMPRCuboid->SetDimensions(dimensions);
	CTMPRCuboid->SetSpacing(spacing);
	CTMPRCuboid->SetOrigin(firstImagePosition);
	CTMPRCuboid->GetPointData()->SetScalars(scalars);
	CTMPRCuboid->GetPointData()->GetScalars()->SetName("CT Cuboid");
	double CTBounds[6];
	CTMPRCuboid->GetBounds(CTBounds);
	RAD_LOG_CRITICAL("<abhishek> bounds:" << CTBounds[0] << ":" << CTBounds[1] << ":" << CTBounds[2] << ":" << CTBounds[3] << ":" << CTBounds[4] << ":" << CTBounds[5]);
	this->initFromImage(CTMPRCuboid);

}
void MPR::initFromDir(string dirPath)
{
	vtkSmartPointer<vtkDICOMImageReader> dicomReader  = vtkSmartPointer<vtkDICOMImageReader>::New();
	dicomReader->SetDirectoryName(dirPath.c_str());
	dicomReader->FileLowerLeftOn();
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

	// scroll slicers to get middle image
	for (int i = 0; i<3; i++)
	{
		d->m_slicers[i]->Scroll(d->m_slicers[i]->GetNumberOfImages() / 2);
	}
}

image MPR::GetOutputImage(Axis axis)
{
	for(int i=0;i<3;i++)
	{
		if(i==axis)
		{
			return d->m_slicers[i]->GetOutputImage();
		}
	}
	return ::born_image();
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

int MPR::GetNumberOfImages(Axis axis)
{
	int numberOfImages = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			numberOfImages = d->m_slicers[i]->GetNumberOfImages();
		}
	}
	return numberOfImages;
}

int MPR::GetCurrentImageIndex(Axis axis)
{
	int idx = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			idx = d->m_slicers[i]->GetSlicerPositionAsIndex();
		}
	}
	return idx;
}

double MPR::GetCurrentImagePosition(Axis axis)
{
	double pos = 0;
	for (int i = 0; i<3; i++)
	{
		if (i == axis)
		{
			pos = d->m_slicers[i]->GetSlicerPosition();
		}
	}
	return pos;
}