// RTDoseMPR.cpp : Defines the entry point for the console application.
//
# pragma comment(lib, "wbemuuid.lib")
#include "stdafx.h"
#include "SOP_UID.h"
#include "rad_logger.h"
#include "rad_template_helper.h"
#include "rad_util.h"
#include "RTDcmtkDicomInterface.h"
#define __FILENAME__ "RTDoseMPR.cpp"
#undef  __MODULENAME__
#define __MODULENAME__ "RTDoseMPR"
using namespace radspeed;
#include "dirent.h"

#include "dcmtk\dcmjpeg\djdecode.h"  /* for dcmjpeg decoders */
#include "dcmtk\dcmjpeg\djencode.h"  /* for dcmjpeg encoders */
#include "dcmtk\dcmdata\dcrledrg.h"  /* for rle decoders */	
#include "dcmtk\dcmjpeg\djrplol.h"   /* for DJ_RPLossless */
#include "dcmtk\dcmjpeg\djrploss.h"  /* for DJ_RPLossy */

// vtk includes
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageAppend.h>
#include <vtkUnsignedIntArray.h>
#include <vtkSignedCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkImageCast.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkCellArray.h>
#include <vtkTransform.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>

#include <vtkJPEGWriter.h>

#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include "vtkInteractorStyleImage.h"
#include "vtkCommand.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkImageMapToWindowLevelColors.h"

#include "vtkDICOMImageReader.h"
#include "vtkOutlineFilter.h"

#include <vtkImageMapToColors.h>
#include <vtkImageFlip.h>

#define CT_SERIES_FOLDER "D:\\Meddiff\\InstaRT\\StreamingData\\1.2.840.113729.1.709.4224.2011.12.2.7.37.14.9.4573\\1.2.840.113729.1.709.4224.2011.12.2.7.37.15.54.4974"

#define DOSE_FILE "D:\\Meddiff\\InstaRT\\StreamingData\\1.2.840.113729.1.709.4224.2011.12.2.7.37.14.9.4573\\1.2.840.113729.1.709.4224.2011.12.2.7.37.15.54.4974\\1.2.246.352.71.7.999955194.12001.20111205145153.dcm"

#include "GL/glut.h"
struct doseFrameData
{
	void* pixelData;
	int frameNumber;
	doseFrameData(){}
};
float FABS(float j)
{
	return fabs(j);
}
#define COLOR 1
vtkImageData* ResampleDose(vtkImageData* image, vtkImageData* referenceImage);
vtkImageData* ResampleDose2(vtkImageData* inputRTImage, vtkImageData* referenceCTImage);
// The mouse motion callback, to turn "Slicing" on and off
class vtkImageInteractionCallback : public vtkCommand
{
public:

  static vtkImageInteractionCallback *New() {
    return new vtkImageInteractionCallback; };
 
  vtkImageInteractionCallback() {
    this->Slicing = 0; 
    this->ImageReslice = 0;
    this->Interactor = 0; };

  void SetImageReslice(vtkImageReslice *reslice) {
    this->ImageReslice = reslice; };

  vtkImageReslice *GetImageReslice() {
    return this->ImageReslice; };

  void SetInteractor(vtkRenderWindowInteractor *interactor) {
    this->Interactor = interactor; };

  vtkRenderWindowInteractor *GetInteractor() {
    return this->Interactor; };

  virtual void Execute(vtkObject *, unsigned long event, void *)
    {
    vtkRenderWindowInteractor *interactor = this->GetInteractor();

    int lastPos[2];
    interactor->GetLastEventPosition(lastPos);
    int currPos[2];
    interactor->GetEventPosition(currPos);
    
    if (event == vtkCommand::LeftButtonPressEvent)
      {
      this->Slicing = 1;
      }
    else if (event == vtkCommand::LeftButtonReleaseEvent)
      {
      this->Slicing = 0;
      }
    else if (event == vtkCommand::MouseMoveEvent)
      {
      if (this->Slicing)
        {
        vtkImageReslice *reslice = this->ImageReslice;

        // Increment slice position by deltaY of mouse
        int deltaY = lastPos[1] - currPos[1];

        reslice->GetOutput()->UpdateInformation();
        double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
        vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
        // move the center point that we are slicing through
        double point[4];
        double center[4];
        point[0] = 0.0;
        point[1] = 0.0;
        point[2] = sliceSpacing * deltaY;
        point[3] = 1.0;
        matrix->MultiplyPoint(point, center);
        matrix->SetElement(0, 3, center[0]);
        matrix->SetElement(1, 3, center[1]);
        matrix->SetElement(2, 3, center[2]);
		cout << center[0]<<":"<<center[1]<<":"<<center[2]<<endl;
        interactor->Render();
        }
      else
        {
        vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(
          interactor->GetInteractorStyle());
        if (style)
          {
          style->OnMouseMove();
          }
        }
      }
    };
 
private: 
  
  // Actions (slicing only, for now)
  int Slicing;

  // Pointer to vtkImageReslice
  vtkImageReslice *ImageReslice;

  // Pointer to the interactor
  vtkRenderWindowInteractor *Interactor;
};

int _tmain(int argc, _TCHAR* argv[])
{
	rad_setLogLevel(255);
	rad_setLogFileName("D:\\RTDoseMPR.log");
	//ResampleDose();

	vector<string> files;
	for(int i=0; i<sopuid_vector.size(); i++)
	{
		string temp = CT_SERIES_FOLDER;
		files.push_back(temp.append("\\").append(sopuid_vector.at(i)).append(".dcm"));
	}
	
	// Inflate RT-Dose
	RTDcmtkDicomInterface* rtDoseDicom=new RTDcmtkDicomInterface("C:\\TestDicom-RT\\RD.1.2.246.352.71.7.999955194.12001.20111205145153.dcm");

	// Pixel spacing
	const char* _pixelSpacing = rtDoseDicom->Get_PIXEL_SPACING();
	vector<string> temp1;
	temp1.clear(); // clear any previously allocated data.
	tokenize(_pixelSpacing, temp1, "\\",true);
	double pixelSpacing[3];
	pixelSpacing[0] = convert_to_double(temp1.at(0).c_str());
	pixelSpacing[1] = convert_to_double(temp1.at(1).c_str());
	pixelSpacing[2] = 2.5;

	// Image patient position
	const char* _ipp = rtDoseDicom->Get_IMAGE_POSITION();
	temp1.clear();
	tokenize(_ipp,temp1,"\\",true);
	double imagePatientPosition[3];

	for(int i=0;i<temp1.size();i++)
	{
		if(i>2)
			continue;
		imagePatientPosition[i]=convert_to_double(temp1.at(i).c_str());
	}
	//imagePatientPosition[2] = imagePatientPosition[2] + 198*2.5;

	double doseGridScaling = atof(rtDoseDicom->Get_DOSEGRID_SCALING());

	
	int width = rtDoseDicom->Get_COLOUMN();
	int height = rtDoseDicom->Get_ROW();
	int dim[3] = { width, height, 1};


	//
	//vector<float> doseGridOffsetVector,ipp;
	//OFString _gridFrameOffsetVector;
	//vector<string> temp;
	//rtDoseDicom->dataset->findAndGetOFStringArray(DCM_GridFrameOffsetVector,_gridFrameOffsetVector);
	//tokenize(_gridFrameOffsetVector.c_str(),temp,"\\");

	//const char* _ipp = rtDoseDicom->Get_IMAGE_POSITION();
	//temp1.clear();
	//tokenize(_ipp,temp1,"\\");

	//for(int i=0;i<temp1.size();i++)
	//{
	//	ipp.push_back(atof(temp1.at(i).c_str()));
	//}
	//// Convert offset vector points from string to float and store permanently.
	//for(int i=0;i<temp.size();i++)
	//{
	//	doseGridOffsetVector.push_back(atof(temp.at(i).c_str()));
	//	//planes.push_back(atof(temp.at(i).c_str())+ipp.at(2));
	//}


	//vector<float> planes;
	//int frame=-1;
	////float z=-238.80;
	//float z=-241.30;

	//for(int i=0;i<doseGridOffsetVector.size();i++)
	//{
	//	planes.push_back(doseGridOffsetVector.at(i)+ipp.at(2));
	//}

	//// Check to see if the requested plane exists in the array
	//std::transform( planes.begin(), planes.end(), planes.begin(), std::bind2nd( std::minus<double>(), z ) );


	//std::transform(planes.begin(),planes.end(),	planes.begin(), FABS);
	////std::for_each(planes.begin(), planes.end(), [](double& d) { d+=z;});
	//vector<float>::iterator it= std::min_element(planes.begin(),planes.end());

	//float zz=*it;
	//cout<< zz;
	//
	//cout<<"this is the plane";
	//int pos = std::find(planes.begin(),planes.end(),zz)-planes.begin();
	
	DJDecoderRegistration::registerCodecs();
	DcmRLEDecoderRegistration::registerCodecs();

	DicomImage * dicomImage = new DicomImage(&rtDoseDicom->file_format, rtDoseDicom->file_format.getDataset()->getOriginalXfer()  , 
		CIF_UsePartialAccessToPixelData, 0,1 /* fcount */);
	
	DJDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();


	vector<doseFrameData*> allFrameDoseData;
	do
	{
		cout<<"processing frame " << dicomImage->getFirstFrame() + 1 << " to "  << dicomImage->getFirstFrame() + dicomImage->getFrameCount()<<endl;
		
		doseFrameData* dfData = new doseFrameData();
		dfData->frameNumber=dicomImage->getFirstFrame();

		// create a vtkImage
		vtkImageData* image = vtkImageData::New();

		image->SetDimensions(width, height, 1);
		image->SetSpacing(pixelSpacing[0],pixelSpacing[1],1);
		image->Initialize();

		vtkDataArray* scalars = 0;

		const DiPixel* diPixel = dicomImage->getInterData();

		switch(diPixel->getRepresentation())
		{
		case EPR_Uint8:
			scalars = vtkUnsignedCharArray::New();
			((vtkUnsignedCharArray*)(scalars))->SetArray(  (unsigned char*)diPixel->getData(), diPixel->getCount(), 1);
			break;

		case EPR_Sint8:
			scalars = vtkSignedCharArray::New();
			((vtkSignedCharArray*)(scalars))->SetArray((signed char*) diPixel->getData(), diPixel->getCount(), 1);
			break;

		case EPR_Uint16:
			scalars = vtkUnsignedShortArray::New();
			((vtkUnsignedShortArray*)(scalars))->SetArray( (unsigned short*) diPixel->getData(), diPixel->getCount(), 1);
			break;

		case EPR_Sint16:
			scalars = vtkShortArray::New();
			((vtkShortArray*)(scalars))->SetArray( (short*)diPixel->getData(), diPixel->getCount(), 1);
			break;

		case EPR_Uint32:
			scalars = vtkUnsignedIntArray::New();
			((vtkUnsignedIntArray*)(scalars))->SetArray( (unsigned int*)diPixel->getData(), diPixel->getCount(), 1);

			break;

		case EPR_Sint32:
			scalars = vtkIntArray::New();
			((vtkIntArray*)(scalars))->SetArray((int*) diPixel->getData(), diPixel->getCount(), 1);
			break;
		default:
			RAD_LOG_CRITICAL("DCMTK EP_Representation type:"<<diPixel->getRepresentation()<<" not supported");
		}

		scalars->SetNumberOfComponents(1);
		image->SetDimensions(width, height, 1);
		image->SetSpacing(pixelSpacing[0],pixelSpacing[1],1);
		//m_vtkImage->SetOrigin(0, 0, 0);

		image->GetPointData()->SetScalars(scalars);
		image->GetPointData()->GetScalars()->SetName("DICOMImage");
		image->Update();
		double bounds[6];
		image->GetBounds(bounds);
		RAD_LOG_CRITICAL("<abhishek> bounds:"<<bounds[0]<<":"<<bounds[1]<<":"<<bounds[2]<<":"<<bounds[3]<<":"<<bounds[4]<<":"<<bounds[5]);
		
		// type cast all images to unsigned int32
		vtkImageCast* imageCast = vtkImageCast::New();
		imageCast->SetInput(image);
		imageCast->SetOutputScalarTypeToUnsignedInt();
		imageCast->Update();

		dfData->pixelData = calloc(diPixel->getCount(),sizeof(U32DataType));
		memcpy(dfData->pixelData,imageCast->GetOutput()->GetPointData()->GetScalars()->GetVoidPointer(0),
			imageCast->GetOutput()->GetPointData()->GetScalars()->GetSize()*sizeof(U32DataType));
		
		imageCast->Delete();
		scalars->Delete();
		image->Delete();

		allFrameDoseData.push_back(dfData);
	}

	while(dicomImage->processNextFrames());

	// create VTK Cuboid
	int dimension[3] = {width, height, allFrameDoseData.size()};
	long dicomSliceSize = dimension[0]*dimension[1];
	long dicomDataSize = dimension[0]*dimension[1]*dimension[2];
	long dicomDataIdx = dicomSliceSize;
	//double spacing[3] = {1.2641,1.2641,2.5};

	void* dicomData = rad_get_memory(dicomDataSize * rad_sizeof(TYPE_U32Data));
	for(int i=0;i<allFrameDoseData.size(); i++)
	{
		U32DataType* dicomData2 = static_cast<U32DataType*>(dicomData);
		dicomData2 += dicomDataSize;
		memcpy(dicomData2-dicomDataIdx, allFrameDoseData.at(i)->pixelData, dicomSliceSize*sizeof(U32DataType));
		dicomDataIdx += dicomSliceSize;
	}
	vtkDataArray* scalars = 0;
	scalars = vtkUnsignedIntArray::New();
	((vtkUnsignedIntArray*)(scalars))->SetArray( (unsigned int*)dicomData, dicomDataSize, 1);
	scalars->SetNumberOfComponents(1);

	vtkImageData* DoseMPRCuboid = vtkImageData::New();
	DoseMPRCuboid->SetDimensions(dimension);
	DoseMPRCuboid->SetSpacing(pixelSpacing);
	DoseMPRCuboid->SetOrigin(imagePatientPosition);
	DoseMPRCuboid->GetPointData()->SetScalars(scalars);
	DoseMPRCuboid->GetPointData()->GetScalars()->SetName("DICOMImage");
	DoseMPRCuboid->Update();
	double bounds[6];
	DoseMPRCuboid->GetBounds(bounds);
	RAD_LOG_CRITICAL("<abhishek> bounds:"<<bounds[0]<<":"<<bounds[1]<<":"<<bounds[2]<<":"<<bounds[3]<<":"<<bounds[4]<<":"<<bounds[5]);


	// append
	/*vtkSmartPointer<vtkImageAppend> imageAppend = vtkSmartPointer<vtkImageAppend>::New();
	for(int i=0;i<allFrameDoseData.size();i++)
	{
		imageAppend->AddInput(allFrameDoseData.at(i)->m_vtkImage);
	}
	imageAppend->Update();*/
	//vtkImageData* combinedDoseImages = imageAppend->GetOutput(); // all dose image combined.

	/*doseFrameData* dfData1 = (doseFrameData*)allFrameDoseData.at(0);*/

	//vtkImageData* combinedDoseImages =  imageCast->GetOutput();
	//// introducing MPR pipe.
	
	int extent[6];
	DoseMPRCuboid->GetExtent(extent);
	int origin[3]={0,0,0};
	double center[3];
    center[0] = origin[0] + pixelSpacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + pixelSpacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + pixelSpacing[2] * 0.5 * (extent[4] + extent[5]);

	static double sagittalElements[16] = {
             0, 0,-1, 0,
             1, 0, 0, 0,
             0,-1, 0, 0,
              0, 0, 0, 1 };

	static double axialElements[16] = {
             1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 };

	static double coronalElements[16] = {
           1, 0, 0, 0,
           0, 0, 1, 0,
           0,-1, 0, 0,
           0, 0, 0, 1 };
	vtkSmartPointer<vtkMatrix4x4> resliceAxes =  vtkMatrix4x4::New();
	resliceAxes->DeepCopy(coronalElements);
    // Set the point through which to slice
    resliceAxes->SetElement(0, 3, center[0]);
    resliceAxes->SetElement(1, 3, center[1]);
    resliceAxes->SetElement(2, 3, center[2]);

	vtkSmartPointer<vtkImageReslice> DoseReslice = vtkImageReslice::New();
	DoseReslice->SetInput(DoseMPRCuboid);
	//DoseReslice->SetResliceAxesOrigin(0,0,0);
	DoseReslice->SetOutputDimensionality(2);
	//DoseReslice->SetOutputSpacing(1, 1 ,1);
	DoseReslice->SetResliceAxes(resliceAxes);
	DoseReslice->InterpolateOff();
	//DoseReslice->SetResliceTransform(sagittalTransform);
	DoseReslice->Update();

	cout << "**********************************************************************"<<endl;
	// create CT MPR
	// create CT MPR cuboid by hand.


	int CTDimension[3] = {512, 512, 199};
	long CTDicomSliceSize = CTDimension[0]*CTDimension[1];
	long CTDicomDataSize = CTDimension[0]*CTDimension[1]*CTDimension[2];
	long CTDicomDataIdx = CTDicomSliceSize;
	double CTSpacing[3] = {1.2641,1.2641,2.5};
	double CTImagePosition[3];



	RadDataType CTDicomDataType = TYPE_NOT_SET;
	void* CTDicomData;
	std::reverse(files.begin(),files.end());
	for(int i=0; i<files.size(); i++)
	{
		cout<< "Processing CT:"<<i<<endl;
		RadRTDicomInterface *pDicom = new RTDcmtkDicomInterface(files.at(i).c_str());	

		if(i==0)
		{
			string imagePosition = string(pDicom->Get_IMAGE_POSITION());
			vector<string> _imgPosition;
			tokenize(imagePosition,_imgPosition,"\\",true);
			for(int i=0; i<_imgPosition.size(); i++)
			{
				CTImagePosition[i] = convert_to_double(_imgPosition.at(i).c_str());
			}

			if(pDicom->Get_BITS_ALLOCATED()/8==1)
			{
				if( !strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(),"RGB") ||
					!strcmp(pDicom->Get_PHOTOMETRIC_INTERPRETATION(),"PALETTE COLOR") )
					CTDicomDataType = TYPE_U32Data;
				else
					CTDicomDataType = TYPE_U8Data;
			}
			else
			{
				if( !pDicom->Get_PIXEL_REPRESENTATION() )
					CTDicomDataType = TYPE_U16Data;
				else
					CTDicomDataType = TYPE_S16Data;
			}
			RAD_LOG_CRITICAL(CTDicomDataType);
			switch(CTDicomDataType)
			{
			case TYPE_U32Data:
				CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U32Data));
				break;
			case TYPE_U8Data:
				CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U8Data));
				break;
			case TYPE_U16Data:
				CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_U16Data));
				break;
			case TYPE_S16Data:
				CTDicomData = rad_get_memory(CTDicomDataSize * rad_sizeof(TYPE_S16Data));
				break;
			}

			CTDicomDataIdx = CTDicomSliceSize;
		}

		//cout << spacing[0]<<" "<< spacing[1]<<" "<< spacing[2]<<endl;
		image pixelData = born_image();
		image overlayPixelData = born_image();
		pDicom->InflateSingleFrameDicomPixelData(&pixelData,&overlayPixelData);
		void* raw_dcm_data = pixelData.data;
	
		switch(CTDicomDataType)
		{
		case TYPE_U32Data: {
			U32DataType* dicomData2 = static_cast<U32DataType*>(CTDicomData);
			dicomData2 += CTDicomDataSize;
			memcpy(dicomData2-CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U32DataType));
			} break;
		case TYPE_U8Data: { 
			U8DataType* dicomData2 = static_cast<U8DataType*>(CTDicomData);
			dicomData2 += CTDicomDataSize;
			memcpy(dicomData2-CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U8DataType));
			} break;
		case TYPE_U16Data: {
			U16DataType* dicomData2 = static_cast<U16DataType*>(CTDicomData);
			dicomData2 += CTDicomDataSize;
			memcpy(dicomData2-CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(U16DataType));
			} break;
		case TYPE_S16Data: {
			S16DataType* dicomData2 = static_cast<S16DataType*>(CTDicomData);
			dicomData2 += CTDicomDataSize;
			memcpy(dicomData2-CTDicomDataIdx, raw_dcm_data, CTDicomSliceSize*sizeof(S16DataType));
			} break;
		}
		CTDicomDataIdx += CTDicomSliceSize;

		if(pixelData.data)
		{
			rad_free_memory(pixelData.data);
		}
		if(overlayPixelData.data)
		{
			rad_free_memory(overlayPixelData.data);
		}

		delete pDicom;
	}

	CTDicomDataIdx -= CTDicomSliceSize;
	vtkDataArray* CTScalars = 0;

	switch(CTDicomDataType)
	{
	case TYPE_U32Data:
		CTScalars = vtkUnsignedIntArray::New();
		((vtkUnsignedIntArray*)(CTScalars))->SetArray( (unsigned int*)CTDicomData, CTDicomDataSize, 1);
		break;
	case TYPE_U8Data:
		scalars = vtkUnsignedCharArray::New();
		((vtkUnsignedCharArray*)(CTScalars))->SetArray( (unsigned char*)CTDicomData, CTDicomDataSize, 1);
		break;
	case TYPE_U16Data:
		CTScalars = vtkUnsignedShortArray::New();
		((vtkUnsignedShortArray*)(CTScalars))->SetArray( (unsigned short*)CTDicomData, CTDicomDataSize, 1);
		break;
	case TYPE_S16Data:
		CTScalars = vtkShortArray::New();
		((vtkShortArray*)(CTScalars))->SetArray( (short*)CTDicomData, CTDicomDataSize, 1);
		break;
	}
	CTScalars->SetNumberOfComponents(1);
	
	
	vtkImageData* CTMPRCuboid = vtkImageData::New();
	CTMPRCuboid->SetDimensions(CTDimension);
	CTMPRCuboid->SetSpacing(CTSpacing);
	//CTMPRCuboid->SetOrigin(CTImagePosition);
	CTMPRCuboid->GetPointData()->SetScalars(CTScalars);
	CTMPRCuboid->GetPointData()->GetScalars()->SetName("CT Cuboid");
	CTMPRCuboid->Update();
	double CTBounds[6];
	CTMPRCuboid->GetBounds(CTBounds);
	RAD_LOG_CRITICAL("<abhishek> bounds:"<<CTBounds[0]<<":"<<CTBounds[1]<<":"<<CTBounds[2]<<":"<<CTBounds[3]<<":"<<CTBounds[4]<<":"<<CTBounds[5]);


	/*vtkSmartPointer<vtkDICOMImageReader> CT_DICOMReader = vtkSmartPointer<vtkDICOMImageReader>::New();
	CT_DICOMReader->FileLowerLeftOn();

	CT_DICOMReader->SetDirectoryName("C:\\TestDicom");
	CT_DICOMReader->Update();

	CT_DICOMReader->GetOutput()->Print(cout);
	double origin1[3];
	CT_DICOMReader->GetOutput()->GetOrigin(origin1);*/
	// set CT MPR pipe
	vtkSmartPointer<vtkImageReslice> CT_Reslice = vtkImageReslice::New();
	CT_Reslice->SetInput(CTMPRCuboid);
	//CT_Reslice->SetResliceAxesOrigin(0,0,0);
	CT_Reslice->SetOutputDimensionality(2);
	CT_Reslice->SetOutputSpacing(1.25,1.25,2.5);
	CT_Reslice->SetResliceAxes(resliceAxes);
	CT_Reslice->InterpolateOff();
	//CT_Reslice->SetResliceTransform(sagittalTransform);
	CT_Reslice->Update();

	//// Create a greyscale lookup table
	vtkLookupTable *CT_Table = vtkLookupTable::New();
	CT_Table->SetRange(0, 2000); // image intensity range
	CT_Table->SetValueRange(0.0, 1.0); // from black to white
	CT_Table->SetSaturationRange(0.0, 0.0); // no color saturation
	CT_Table->SetRampToLinear();
	CT_Table->Build();

	//// Map the image through the lookup table
	vtkImageMapToWindowLevelColors *CT_Color = vtkImageMapToWindowLevelColors::New();
	CT_Color->SetInputConnection(CT_Reslice->GetOutputPort());
	CT_Color->SetOutputFormatToLuminance();
	CT_Color->SetInput( CT_Reslice->GetOutput());
    CT_Color->SetWindow(719.0);
	CT_Color->SetLevel(8.0);

//	vtkImageData* resampled = ResampleDose2(DoseReslice->GetOutput(),imageCast->GetOutput());


	/*112::1.064e+006
	102::969000
	100::950000
	98::931000
	95::902500
	90::855000
	80::760000
	70::665000
	50::475000
	30::285000*/
	
	// generate contour
	double contourLevel1 = 285000; // 30%
	double contourLevel2 = 475000; // 50%
	double contourLevel3 = 665000; // 70%
	double contourLevel4 = 760000; // 80%
	double contourLevel5 = 855000; // 90%

	vtkImageData* image = DoseReslice->GetOutput();
	vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
	contourFilter->SetInput(DoseReslice->GetOutput());
	contourFilter->GenerateValues(1,contourLevel1,contourLevel1);
	contourFilter->Update();



#ifdef COLOR
	vtkLookupTable *RT_LUT_Table = vtkLookupTable::New();

	RT_LUT_Table->SetRange(285000, 760000); // image intensity range
	RT_LUT_Table->SetNumberOfColors(7);
    double opacity = 0.3;
    RT_LUT_Table->SetTableValue(0, 0, 0, 1, opacity);
    RT_LUT_Table->SetTableValue(1, 0, 1.0, 0, opacity);
    RT_LUT_Table->SetTableValue(2, 0.6, 1.0, 0.0, opacity);
    RT_LUT_Table->SetTableValue(3, 1.0, 1.0, 0.0, 0.7);
    RT_LUT_Table->SetTableValue(4, 1.0, 0.8, 0.0, opacity);
    RT_LUT_Table->SetTableValue(5, 1.0, 0.4, 0.0, opacity);
    RT_LUT_Table->SetTableValue(6, 1.0, 0.0, 0.0, 1);
	
	//// Map the image through the lookup table
	vtkImageMapToColors *RT_Color = vtkImageMapToColors::New();
	RT_Color->SetInput( DoseReslice->GetOutput());
	RT_Color->SetLookupTable(RT_LUT_Table);
	RT_Color->Update();

	vtkImageActor *ColoredDose_Actor = vtkImageActor::New();
	ColoredDose_Actor->SetInput(RT_Color->GetOutput());
#endif
	// visualize contour
	vtkPolyData* polydata = contourFilter->GetOutput();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInput(polydata);

	vtkCellArray* verts = contourFilter->GetOutput()->GetLines();
	vtkPoints* pPoints = contourFilter->GetOutput()->GetPoints();
	vtkIdType nPts = 0;
	vtkIdType *ptIds = verts->GetPointer();
	double v[3];
	string fileName1 = "D:\\ROITest\\";
	fileName1.append("RTDoseMPR_Slice_Points_DicomOrigin");
	fileName1.append(".csv");
	ofstream MyFile;
	MyFile.open (fileName1.c_str(), ios::out | ios::ate | ios::app | ios::binary) ;
	MyFile << "x, y, z"<<endl;

	for(verts->InitTraversal(); verts->GetNextCell(nPts,ptIds);)
	{
		for (int j = 0; j < nPts; j++) 
		{
			pPoints->GetPoint(ptIds[j], v);
			cout <<v[0]<<","<<v[1]<<endl;
			MyFile << v[0] << "," << v[1] << "," << v[2] <<endl;
			//::glNormal3f (n[0], n[1], n[2]);
			//::glVertex3f (v[0], v[1], v[2]);
		}
	}
	MyFile.close();

//	mapper->SetLookupTable(RT_LUT_Table);


	vtkSmartPointer<vtkActor> DoseActor = vtkSmartPointer<vtkActor>::New();
	DoseActor->SetMapper(mapper);
	//DoseActor->GetProperty()->SetEdgeVisibility(1);
	DoseActor->GetProperty()->SetEdgeColor(1,0,0);

	// create dose outline
	vtkSmartPointer<vtkOutlineFilter> doseOutline = vtkSmartPointer<vtkOutlineFilter>::New();
	doseOutline->SetInput(DoseReslice->GetOutput());
	doseOutline->Update();

	vtkSmartPointer<vtkPolyDataMapper> OutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	OutlineMapper->SetInput(doseOutline->GetOutput());

	vtkSmartPointer<vtkActor> OutlineActor = vtkSmartPointer<vtkActor>::New();
	OutlineActor->SetMapper(OutlineMapper);
	//OutlineActor->GetProperty()->SetEdgeVisibility(1);
	//OutlineActor->GetProperty()->SetEdgeColor(1,0,0);


	// Display the image
	vtkImageActor *CT_Actor = vtkImageActor::New();
	CT_Actor->SetInput(CT_Color->GetOutput());

	vtkSmartPointer<vtkOutlineFilter> CTOutline = vtkSmartPointer<vtkOutlineFilter>::New();
	CTOutline->SetInput(CT_Reslice->GetOutput());
	CTOutline->Update();

	vtkSmartPointer<vtkPolyDataMapper> CTOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	CTOutlineMapper->SetInput(CTOutline->GetOutput());

	vtkSmartPointer<vtkActor> CTOutlineActor = vtkSmartPointer<vtkActor>::New();
	CTOutlineActor->SetMapper(CTOutlineMapper);
	CTOutlineActor->GetProperty()->SetEdgeVisibility(1);
	CTOutlineActor->GetProperty()->SetEdgeColor(0,1,0);


	// Visualize
	vtkSmartPointer<vtkRenderer> renderer =	vtkSmartPointer<vtkRenderer>::New();
	
	vtkSmartPointer<vtkRenderWindow> renderWindow =	vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->AddActor(DoseActor);
	renderer->AddActor(OutlineActor);

#ifdef COLOR
	//renderer->AddActor(ColoredDose_Actor);
#endif
//	renderer->AddActor(CT_Actor);
//	renderer->AddActor(CTOutlineActor);

	renderWindow->Render();

	// Set up the interaction
	vtkInteractorStyleImage *imageStyle = vtkInteractorStyleImage::New();
	renderWindowInteractor->SetInteractorStyle(imageStyle);
	renderWindow->SetInteractor(renderWindowInteractor);
	renderWindow->Render();

	vtkImageInteractionCallback *callback = vtkImageInteractionCallback::New();
	callback->SetImageReslice(DoseReslice);
	callback->SetInteractor(renderWindowInteractor);

	imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
	imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
	imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);

	renderWindowInteractor->Start();

	return 0;
}


//#include "itkImage.h"
//#define ITK_IO_FACTORY_REGISTER_MANAGER
//#include "itkImageFileWriter.h"
//#include "itkImageFileReader.h"
//
//#include "itkVTKImageToImageFilter.h"
//#include "itkImageToVTKImageFilter.h"
//#include "itkResampleImageFilter.h"
//#include "itkAffineTransform.h"
//#include "itkLinearInterpolateImageFunction.h"
//#include "itkMultiResolutionImageRegistrationMethod.h"
//#include "itkTranslationTransform.h"
//#include "itkMattesMutualInformationImageToImageMetric.h"
//#include "itkRegularStepGradientDescentOptimizer.h"
//#include "itkCenteredTransformInitializer.h"
//
//// Software Guide : BeginCodeSnippet
//#include "itkAffineTransform.h"
//// Software Guide : EndCodeSnippet
//#include "itkCenteredTransformInitializer.h"
//#include "itkMultiResolutionImageRegistrationMethod.h"
//#include "itkMattesMutualInformationImageToImageMetric.h"
//#include "itkRegularStepGradientDescentOptimizer.h"
//#include "itkRecursiveMultiResolutionPyramidImageFilter.h"
//#include "itkImage.h"
//
//#include "vtkPNGReader.h"
//#include "vtkPNGWriter.h"
//#include "vtkImageLuminance.h"
//
////#define RESAMPLE_PROTO 1
////  The following section of code implements an observer
////  that will monitor the evolution of the registration process.
////
//#include "itkCommand.h"
//class CommandIterationUpdate : public itk::Command
//{
//public:
//  typedef  CommandIterationUpdate   Self;
//  typedef  itk::Command             Superclass;
//  typedef  itk::SmartPointer<Self>  Pointer;
//  itkNewMacro( Self );
//protected:
//  CommandIterationUpdate(): m_CumulativeIterationIndex(0) {};
//public:
//  typedef   itk::RegularStepGradientDescentOptimizer  OptimizerType;
//  typedef   const OptimizerType *                     OptimizerPointer;
//  void Execute(itk::Object *caller, const itk::EventObject & event)
//    {
//    Execute( (const itk::Object *)caller, event);
//    }
//  void Execute(const itk::Object * object, const itk::EventObject & event)
//    {
//    OptimizerPointer optimizer =
//      dynamic_cast< OptimizerPointer >( object );
//    if( !(itk::IterationEvent().CheckEvent( &event )) )
//      {
//      return;
//      }
//    std::cout << optimizer->GetCurrentIteration() << "   ";
//    std::cout << optimizer->GetValue() << "   ";
//    std::cout << optimizer->GetCurrentPosition() << "  " <<
//      m_CumulativeIterationIndex++ << std::endl;
//    }
//private:
//  unsigned int m_CumulativeIterationIndex;
//};
////  The following section of code implements a Command observer
////  that will control the modification of optimizer parameters
////  at every change of resolution level.
////
//template <typename TRegistration>
//class RegistrationInterfaceCommand : public itk::Command
//{
//public:
//  typedef  RegistrationInterfaceCommand   Self;
//  typedef  itk::Command                   Superclass;
//  typedef  itk::SmartPointer<Self>        Pointer;
//  itkNewMacro( Self );
//protected:
//  RegistrationInterfaceCommand() {};
//public:
//  typedef   TRegistration                              RegistrationType;
//  typedef   RegistrationType *                         RegistrationPointer;
//  typedef   itk::RegularStepGradientDescentOptimizer   OptimizerType;
//  typedef   OptimizerType *                            OptimizerPointer;
//  void Execute(itk::Object * object, const itk::EventObject & event)
//  {
//    if( !(itk::IterationEvent().CheckEvent( &event )) )
//      {
//      return;
//      }
//    RegistrationPointer registration =
//                        dynamic_cast<RegistrationPointer>( object );
//    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
//                       registration->GetModifiableOptimizer() );
//    std::cout << "-------------------------------------" << std::endl;
//    std::cout << "MultiResolution Level : "
//              << registration->GetCurrentLevel()  << std::endl;
//    std::cout << std::endl;
//    if ( registration->GetCurrentLevel() == 0 )
//      {
//      optimizer->SetMaximumStepLength( 16.00 );
//      optimizer->SetMinimumStepLength(  0.01 );
//      }
//    else
//      {
//      optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() / 4.0 );
//      optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() / 10.0 );
//      }
//  }
//  void Execute(const itk::Object * , const itk::EventObject & )
//    { return; }
//};
//
//vtkImageData* ResampleDose2(vtkImageData* inputRTImage, vtkImageData* referenceCTImage)
//{
//	const unsigned int Dimension = 2;
//	typedef   unsigned int  ReferenceCTPixelType;
//	typedef   unsigned int  InputRTImagePixelType;
//	typedef   unsigned int  OutputPixelType;
//
//	typedef itk::Image< ReferenceCTPixelType,  Dimension >   ReferenceCTImageType;
//	typedef itk::Image< InputRTImagePixelType, Dimension >   InputRTImageType;
//	typedef itk::Image< OutputPixelType, Dimension >   OutputImageType;
//
//	// convert vtk image to itk image
//	typedef itk::VTKImageToImageFilter<InputRTImageType> InputRTVTKImageToImageType;
// 
//	// convert input RT image
//	InputRTVTKImageToImageType::Pointer InputRTVTKImageToITKImageFilter = InputRTVTKImageToImageType::New();
//	InputRTVTKImageToITKImageFilter->SetInput(inputRTImage);
//	InputRTVTKImageToITKImageFilter->Update();
//
//	// convert fixed image
//	typedef itk::VTKImageToImageFilter<ReferenceCTImageType> ReferenceCTImageVTKImageToImageType;
//	ReferenceCTImageVTKImageToImageType::Pointer ReferenceCTImageVTKImageToITKImageFilter = ReferenceCTImageVTKImageToImageType::New();
//	ReferenceCTImageVTKImageToITKImageFilter->SetInput(referenceCTImage);
//	ReferenceCTImageVTKImageToITKImageFilter->Update();
//
//	// create resample filter
//	typedef itk::ResampleImageFilter<InputRTImageType,OutputImageType> FilterType;
//	FilterType::Pointer filter = FilterType::New();
//
//	// create transform
//	typedef itk::TranslationTransform< double, Dimension >  TransformType;
//
//	TransformType::Pointer transform = TransformType::New();
//	filter->SetTransform( transform );
//  
//
//	typedef itk::LinearInterpolateImageFunction<InputRTImageType, double >  InterpolatorType;
//	InterpolatorType::Pointer interpolator = InterpolatorType::New();
//	filter->SetInterpolator( interpolator );
//
//	filter->SetDefaultPixelValue( 100 );
//
//	filter->UseReferenceImageOn();
//	filter->SetReferenceImage(ReferenceCTImageVTKImageToITKImageFilter->GetOutput());
//	filter->SetInput( InputRTVTKImageToITKImageFilter->GetOutput() );
//	filter->UpdateLargestPossibleRegion();
//	filter->Print(cout);
// 
//	OutputImageType::Pointer output = filter->GetOutput();
//
//	typedef itk::ImageToVTKImageFilter<OutputImageType> ImageToVTKImageType;
//	ImageToVTKImageType::Pointer itkImageToVTKImageFilter = ImageToVTKImageType::New();
//
//	itkImageToVTKImageFilter->SetInput(output);
//	itkImageToVTKImageFilter->Update();
//
//	vtkImageData* result1 = itkImageToVTKImageFilter->GetOutput();
//	
//	vtkImageData* grayVTK = vtkImageData::New();
//	
//	grayVTK->SetExtent(result1->GetExtent());
//	grayVTK->SetSpacing(result1->GetSpacing());
//	grayVTK->SetScalarTypeToUnsignedInt();
//	grayVTK->SetNumberOfScalarComponents(1);
//	grayVTK->AllocateScalars();
//	grayVTK->DeepCopy(static_cast<vtkImageData*>(itkImageToVTKImageFilter->GetOutput()));
//
//	return grayVTK;
//
//
//}
////vtkImageData* ResampleDose(vtkImageData* movingImage, vtkImageData* fixedImage)
////{
////	const unsigned int Dimension = 2;
////	typedef   unsigned int  FixedImagePixelType;
////	typedef   unsigned int  MovingImagePixelType;
////
////	typedef itk::Image< FixedImagePixelType,  Dimension >   FixedImageType;
////	typedef itk::Image< MovingImagePixelType, Dimension >   MovingImageType;
////
////	// convert vtk image to itk image
////	typedef itk::VTKImageToImageFilter<FixedImageType> VTKImageToImageType;
//// 
////	// convert moving image
////	VTKImageToImageType::Pointer movingVTKImageToITKImageFilter = VTKImageToImageType::New();
////	movingVTKImageToITKImageFilter->SetInput(movingImage);
////	movingVTKImageToITKImageFilter->Update();
////
////	// convert fixed image
////	VTKImageToImageType::Pointer fixedVTKImageToITKImageFilter = VTKImageToImageType::New();
////	fixedVTKImageToITKImageFilter->SetInput(fixedImage);
////	fixedVTKImageToITKImageFilter->Update();
////
////	//
////	typedef float InternalPixelType;
////	typedef itk::Image< InternalPixelType, Dimension > InternalImageType;
////
////	//  The types for the registration components are then derived using
////	//  the internal image type.
////	typedef itk::AffineTransform< double, Dimension > TransformType;
////	typedef itk::RegularStepGradientDescentOptimizer       OptimizerType;
////	typedef itk::LinearInterpolateImageFunction< InternalImageType, double > InterpolatorType;
////	typedef itk::MattesMutualInformationImageToImageMetric< InternalImageType, InternalImageType > MetricType;
////	typedef OptimizerType::ScalesType       OptimizerScalesType;
////
////	typedef itk::MultiResolutionImageRegistrationMethod< InternalImageType, InternalImageType > RegistrationType;
////
////
////
////	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType > FixedImagePyramidType;
////	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType > MovingImagePyramidType;
////
////	//  All the components are instantiated using their New() method
////	//  and connected to the registration object
////	TransformType::Pointer      transform     = TransformType::New();
////	OptimizerType::Pointer      optimizer     = OptimizerType::New();
////	InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
////	RegistrationType::Pointer   registration  = RegistrationType::New();
////	MetricType::Pointer         metric        = MetricType::New();
////
////	FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
////	MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();
////
////	registration->SetOptimizer(     optimizer     );
////	registration->SetTransform(     transform     );
////	registration->SetInterpolator(  interpolator  );
////	registration->SetMetric( metric  );
////	registration->SetFixedImagePyramid( fixedImagePyramid );
////	registration->SetMovingImagePyramid( movingImagePyramid );
////
////	typedef itk::CastImageFilter< FixedImageType, InternalImageType >  FixedCastFilterType;
////	typedef itk::CastImageFilter< MovingImageType, InternalImageType > MovingCastFilterType;
////
////	FixedCastFilterType::Pointer fixedCaster   = FixedCastFilterType::New();
////	MovingCastFilterType::Pointer movingCaster = MovingCastFilterType::New();
////
////	fixedCaster->SetInput(  fixedVTKImageToITKImageFilter->GetOutput() );
////	movingCaster->SetInput( movingVTKImageToITKImageFilter->GetOutput() );
////
////	registration->SetFixedImage(    fixedCaster->GetOutput()    );
////	registration->SetMovingImage(   movingCaster->GetOutput()   );
////
////	fixedCaster->Update();
////
////	registration->SetFixedImageRegion( fixedCaster->GetOutput()->GetBufferedRegion() );
////
////	 typedef itk::CenteredTransformInitializer< TransformType, FixedImageType, MovingImageType >  TransformInitializerType;
////	TransformInitializerType::Pointer initializer = TransformInitializerType::New();
////	initializer->SetTransform(   transform );
////	initializer->SetFixedImage(  fixedVTKImageToITKImageFilter->GetOutput() );
////	initializer->SetMovingImage( fixedVTKImageToITKImageFilter->GetOutput() );
////	initializer->MomentsOn();
////	initializer->InitializeTransform();
////	registration->SetInitialTransformParameters( transform->GetParameters() );
////
////	OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
////	optimizerScales[0] = 1.0; // scale for M11
////	optimizerScales[1] = 1.0; // scale for M12
////	optimizerScales[2] = 1.0; // scale for M21
////	optimizerScales[3] = 1.0; // scale for M22
////	optimizerScales[4] = 1.0 / 1e7; // scale for translation on X
////	optimizerScales[5] = 1.0 / 1e7; // scale for translation on Y
////
////	 optimizer->SetScales( optimizerScales );
////
////	//typedef RegistrationType::ParametersType ParametersType;
////	//ParametersType initialParameters( transform->GetNumberOfParameters() );
////
////	//initialParameters[0] = 0.0;  // Initial offset in mm along X
////	//initialParameters[1] = 0.0;  // Initial offset in mm along Y
////
////	//registration->SetInitialTransformParameters( initialParameters );
////
////	metric->SetNumberOfHistogramBins( 128 );
////	metric->SetNumberOfSpatialSamples( 5 );
////
////	metric->ReinitializeSeed( 76926294 );
////
////	optimizer->MinimizeOn();
////	optimizer->SetMaximumStepLength( 1.00 );
////	optimizer->SetMinimumStepLength( 0.001 );
////
////	optimizer->SetNumberOfIterations( 200 );
////	optimizer->SetRelaxationFactor( 0.8 );
////
////	// Create the Command observer and register it with the optimizer.
////  //
////  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
////  optimizer->AddObserver( itk::IterationEvent(), observer );
////  // Create the Command interface observer and register it with the optimizer.
////  //
////  typedef RegistrationInterfaceCommand<RegistrationType> CommandType;
////  CommandType::Pointer command = CommandType::New();
////  registration->AddObserver( itk::IterationEvent(), command );
////
////	registration->SetNumberOfLevels( 3 );
////
////	try
////	{
////		registration->Update();
////		std::cout << "Optimizer stop condition: "
////				  << registration->GetOptimizer()->GetStopConditionDescription()
////				  << std::endl;
////	}
////	catch( itk::ExceptionObject & err )
////	{
////		std::cout << "ExceptionObject caught !" << std::endl;
////		std::cout << err << std::endl;
////		return NULL;
////	}
////
////	typedef RegistrationType::ParametersType ParametersType;
////	ParametersType finalParameters = registration->GetLastTransformParameters();
////
////	double TranslationAlongX = finalParameters[0];
////	double TranslationAlongY = finalParameters[1];
////
////	unsigned int numberOfIterations = optimizer->GetCurrentIteration();
////
////	double bestValue = optimizer->GetValue();
////
////
////	// Print out results
////	//
////	std::cout << "Result = " << std::endl;
////	std::cout << " Translation X = " << TranslationAlongX  << std::endl;
////	std::cout << " Translation Y = " << TranslationAlongY  << std::endl;
////	std::cout << " Iterations    = " << numberOfIterations << std::endl;
////	std::cout << " Metric value  = " << bestValue          << std::endl;
////
////
////	typedef itk::ResampleImageFilter< MovingImageType, FixedImageType > ResampleFilterType;
////
////	TransformType::Pointer finalTransform = TransformType::New();
////
////	finalTransform->SetParameters( finalParameters );
////	finalTransform->SetFixedParameters( transform->GetFixedParameters() );
////
////	ResampleFilterType::Pointer resample = ResampleFilterType::New();
////
////	resample->SetTransform( finalTransform );
////	resample->SetInput( movingVTKImageToITKImageFilter->GetOutput() );
////
////	FixedImageType::Pointer fixedITKImage = fixedVTKImageToITKImageFilter->GetOutput();
////
////
////	resample->SetSize(    fixedITKImage->GetLargestPossibleRegion().GetSize() );
////	resample->SetOutputOrigin(  fixedITKImage->GetOrigin() );
////	resample->SetOutputSpacing( fixedITKImage->GetSpacing() );
////	resample->SetOutputDirection( fixedITKImage->GetDirection() );
////	resample->SetDefaultPixelValue( 100 );
////	resample->Update();
////
////	FixedImageType::Pointer output = resample->GetOutput();
////
////	typedef  unsigned int  OutputPixelType;
////
////	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
////
////	typedef itk::CastImageFilter< FixedImageType,  OutputImageType > CastFilterType;
////	CastFilterType::Pointer  caster =  CastFilterType::New();
////	caster->SetInput( resample->GetOutput() );
////
////	caster->Update();
////
////	typedef itk::ImageToVTKImageFilter<FixedImageType> ImageToVTKImageType;
////	ImageToVTKImageType::Pointer itkImageToVTKImageFilter = ImageToVTKImageType::New();
////
////	itkImageToVTKImageFilter->SetInput(caster->GetOutput());
////	itkImageToVTKImageFilter->Update();
////
////	vtkImageData* result1 = itkImageToVTKImageFilter->GetOutput();
////
////	vtkImageData* grayVTK = vtkImageData::New();
////	
////	grayVTK->SetExtent(result1->GetExtent());
////	grayVTK->SetSpacing(result1->GetSpacing());
////	grayVTK->SetScalarTypeToUnsignedInt();
////	grayVTK->SetNumberOfScalarComponents(1);
////	grayVTK->AllocateScalars();
////	grayVTK->DeepCopy(static_cast<vtkImageData*>(itkImageToVTKImageFilter->GetOutput()));
////
////	return grayVTK;
////
////
////#ifdef RESAMPLE_PROTO
////	
////	
////	/*typedef itk::ImageFileReader< InputImageType  >  ReaderType;
////	typedef itk::ImageFileWriter< OutputImageType >  WriterType;*/
////
////	/*ReaderType::Pointer reader = ReaderType::New();
////	WriterType::Pointer writer = WriterType::New();
////
////	reader->SetFileName( "D:\\Image1.png" );
////	writer->SetFileName( "D:\\Image1_ITK2.png" );*/
////
////	typedef itk::ImageToVTKImageFilter<OutputImageType> ImageToVTKImageType;
////	ImageToVTKImageType::Pointer itkImageToVTKImageFilter = ImageToVTKImageType::New();
////
////	typedef itk::ResampleImageFilter<InputImageType,OutputImageType> FilterType;
////	FilterType::Pointer filter = FilterType::New();
////
////	typedef itk::AffineTransform< double, Dimension >  TransformType;
////
////	TransformType::Pointer transform = TransformType::New();
////	filter->SetTransform( transform );
////  
////
////	typedef itk::LinearInterpolateImageFunction<InputImageType, double >  InterpolatorType;
////	InterpolatorType::Pointer interpolator = InterpolatorType::New();
////	filter->SetInterpolator( interpolator );
////
////	filter->SetDefaultPixelValue( 100 );
////
//////	const double spacing[ Dimension ] = {p_spacing[0], p_spacing[1]};
//////	filter->SetOutputSpacing( spacing );
////	
////	//// Physical space coordinate of origin for X and Y
////	//const double origin[ Dimension ] = { p_origin[0],p_origin[1]};
////	//filter->SetOutputOrigin( origin );
////
////	//InputImageType::DirectionType direction;
////	//direction.SetIdentity();
////	//filter->SetOutputDirection( direction );
////
////	//InputImageType::SizeType   size;
////
////	//size[0] = p_dim[0];  // number of pixels along X
////	//size[1] = p_dim[1];  // number of pixels along Y
////
////	//TransformType::OutputVectorType translation;
////	//translation[0] = p_center[0] * -1;  // X translation in millimeters
////	//translation[1] = p_center[1] * -1;  // Y translation in millimeters
////	//transform->Translate( translation );
////
////	//filter->SetSize( size );
////	filter->UseReferenceImageOn();
////	filter->SetReferenceImage(referenceImageToImageFilter->GetOutput());
////	filter->SetInput( vtkImageToImageFilter->GetOutput() );
////	filter->UpdateLargestPossibleRegion();
////
////	
////
//// 
////	OutputImageType::Pointer output = filter->GetOutput();
////
////	//writer->SetInput( output );
////	//writer->Update();
////
////	itkImageToVTKImageFilter->SetInput(output);
////	itkImageToVTKImageFilter->Update();
////
////	/*vtkSmartPointer<vtkImageData> grayVTK = vtkSmartPointer<vtkImageData>::New();
////	grayVTK->SetExtent();
////	grayVTK->SetSpacing(m_spacing);
////	grayVTK->SetScalarTypeToUnsignedShort();
////	grayVTK->SetNumberOfScalarComponents(1);
////	grayVTK->AllocateScalars();
////	grayVTK->DeepCopy(static_cast<vtkImageData*>(itk2vtkGray->GetOutput()));*/
////
////	vtkImageData* result1 = itkImageToVTKImageFilter->GetOutput();
////	
////	vtkImageData* grayVTK = vtkImageData::New();
////	
////	grayVTK->SetExtent(result1->GetExtent());
////	grayVTK->SetSpacing(result1->GetSpacing());
////	grayVTK->SetScalarTypeToUnsignedInt();
////	grayVTK->SetNumberOfScalarComponents(1);
////	grayVTK->AllocateScalars();
////	grayVTK->DeepCopy(static_cast<vtkImageData*>(itkImageToVTKImageFilter->GetOutput()));
////
////	return grayVTK;
////
////#else
////	
////#endif
////	
////}