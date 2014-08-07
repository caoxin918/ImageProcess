#include "imageprocess.h"
#include "QFileDialog"
#include "QMessageBox"
#include <stdio.h>

using namespace std;

ImageProcess::ImageProcess(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	QObject::connect(ui.pushButtonPhotograph,SIGNAL(clicked()),this,SLOT(on_pushButton_photograph_clicked()));
	QObject::connect(ui.pushButtonLuminescence,SIGNAL(clicked()),this,SLOT(on_pushButton_luminescence_clicked()));
	QObject::connect(ui.pushButtonSubstract,SIGNAL(clicked()),this,SLOT(on_pushButton_substract_clicked()));
	QObject::connect(ui.pushButtonSmooth,SIGNAL(clicked()),this,SLOT(on_pushButton_smooth_clicked()));
	QObject::connect(ui.pushButtonFusion,SIGNAL(clicked()),this,SLOT(on_pushButton_fusion_clicked()));
	QObject::connect(ui.pushButtonSave,SIGNAL(clicked()),this,SLOT(on_pushButton_save_clicked()));
	QObject::connect(ui.pushButtonClear,SIGNAL(clicked()),this,SLOT(on_pushButton_clear_clicked()));
	QObject::connect(ui.pushButtonQuit,SIGNAL(clicked()),this,SLOT(on_pushButton_quit_clicked()));
	QObject::connect(ui.spinBoxHighValue,SIGNAL(valueChanged(int)),this,SLOT(on_spinBoxValueChanged()));
	QObject::connect(ui.spinBoxLowValue,SIGNAL(valueChanged(int)),this,SLOT(on_spinBoxValueChanged()));
	
	QRegExp regx("[0-9]+$");
	QValidator *validator1 = new QRegExpValidator(regx,ui.SubstractLineEdit);
	ui.SubstractLineEdit->setValidator(validator1);
	QValidator *validator2 = new QRegExpValidator(regx,ui.SmoothLineEdit);
	ui.SmoothLineEdit->setValidator(validator2);

	initialAll();
}

ImageProcess::~ImageProcess()
{

}

void ImageProcess::initialAll()
{
	luminescenceImage=ImageType::New();
	photographImage=UnChImageType::New();
	tempImageData=ImageType::New();
	tiffIO=TIFFIOType::New();

	rgbImageData=RGBImageType::New();
	filter=MedianFilterType::New();

	connector=ConnectorType::New();
	viewer=vtkImageViewer::New();
	viewer2=vtkImageViewer2::New();
	renderer=vtkRenderer::New();
	imageFlip=vtkImageFlip::New();
	//ui.qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
	//renderWindowInteractor=vtkRenderWindowInteractor::New();
	//renderer->

	ui.qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
	renderWindowInteractor=ui.qvtkWidget->GetRenderWindow()->GetInteractor();
	renderWindowInteractor->Disable();

	photographFlag=false;
	luminescenceFlag=false;
	substractFlag=false;
	smoothFilterFlag=false;
	fusionFlag=false;
	substractValue=0;
	smoothFilterValue=1;
	colorbarLowValue=0;
	colorbarHighValue=1;
	colorLevel=1;
	colorWindow=2;
	highValue=1;
	lowValue=0;
}

void ImageProcess::on_pushButton_photograph_clicked()
{
	QString path=QFileDialog::getOpenFileName(this,"Open photograph",".","tiff Files(*.tif)");
	if(path.length()==0){
		QMessageBox::information(NULL,"Warning","You didn't select any files.");
		if(photographFlag==true)
		{
			photographImage=NULL;;
			photographImage=UnChImageType::New();
			clearImageShow();
		}
		photographFlag=false;
	}else
	{
		photographFlag=true;
		string temp1=path.toStdString();
		const char* temp2=temp1.c_str();
		photographImage=NULL;;
		photographImage=UnChImageType::New();
		photographImage=rescaleImage(readImage(temp2),0,255);
		
		////////////////////////////////////////
// 		vtkTIFFReader* VTKreader=vtkTIFFReader::New();
// 		VTKreader->SetFileName("tempPseudocolorFile.tif");
// 		VTKreader->Update();
// 		imageFlip->SetInput(VTKreader->GetOutput());
// 		imageFlip->SetFilteredAxis(1);
// 		imageFlip->Update();
// 		renderer->RemoveAllViewProps();;
//  		viewer2=vtkImageViewer2::New();
// 		viewer2->SetInput(imageFlip->GetOutput());
// 		viewer2->UpdateDisplayExtent();
// 		viewer2->SetRenderWindow(ui.qvtkWidget->GetRenderWindow());
// 		viewer2->SetRenderer(renderer);
// 		viewer2->SetPosition(0,0);
// 		renderer->ResetCamera();
// 		renderer->DrawOn();
// 		ui.qvtkWidget->GetRenderWindow()->Render();

		///////////////

//		imageShow(photographImage);
	}
	
}
void ImageProcess::on_pushButton_luminescence_clicked()
{
	QString path=QFileDialog::getOpenFileName(this,"Open Luminescence image",".","tiff Files(*.tif)");
	if(path.length()==0){
		QMessageBox::information(NULL,"Warning","You didn't select any files.");
		if(luminescenceFlag==true)
		{
			substractFlag=false;
			smoothFilterFlag=false;
			fusionFlag=false;
			luminescenceImage=NULL;
			luminescenceImage=ImageType::New();
			clearImageShow();
		}
		luminescenceFlag=false;
	}else
	{
		luminescenceFlag=true;
		string temp1=path.toStdString();
		const char* temp2=temp1.c_str();
		luminescenceImage=readImage(temp2);

		substractFlag=false;
		smoothFilterFlag=false;
		fusionFlag=false;

		imageShow(luminescenceImage);
	}
}
void ImageProcess::on_pushButton_substract_clicked()
{
	unsigned long temp;
	temp=ui.SubstractLineEdit->text().toULong();
 	if(!luminescenceFlag)
	{
		QMessageBox::information(NULL,"Warning","You didn't select a luminescence image.");
		substractFlag=false;
		substractFlag=false;
		smoothFilterFlag=false;
		fusionFlag=false;
		return;
	}
	if(temp>65535)
	{
		QMessageBox::information(NULL,"Warning","The input must be smaller than 65535.");
		substractFlag=false;
		substractFlag=false;
		smoothFilterFlag=false;
		fusionFlag=false;
		return;
	}
	substractValue=temp;
	//process image, substract step
	tempImageData=NULL;//the first temp image data
	tempImageData=ImageType::New();
	copyImageData(luminescenceImage,tempImageData);//tempImagedata is used as the input data for the substract step
	IteratorType It(tempImageData,tempImageData->GetRequestedRegion());
	while(!It.IsAtEnd())
	{
		PixelType temp;
		temp=(minusPixel(It.Get(),substractValue));
		It.Set(temp);
		++It;
	}
	imageShow(tempImageData);
//	writeImage(tempImageData,"tesss.tif");
	substractFlag=true;

	smoothFilterFlag=false;
	fusionFlag=false;

}
void ImageProcess::on_pushButton_smooth_clicked()
{
	unsigned long temp;
	temp=ui.SmoothLineEdit->text().toULong();
	if(!substractFlag)
	{
		QMessageBox::information(NULL,"Warning","Please substract the background first.");
		smoothFilterFlag=false;
		smoothFilterFlag=false;
		fusionFlag=false;
		return;
	}
	if(temp<1 || temp>20)
	{
		QMessageBox::information(NULL,"Warning","The input must be larger than 0 and smaller than 10.");
		smoothFilterFlag=false;
		smoothFilterFlag=false;
		fusionFlag=false;
		return;
	}
	smoothFilterValue=temp;
	tempImageData2=NULL;
	tempImageData2=ImageType::New();
	//copyImageData(tempImageData,tempImageData2);//tempImageData2 is used for the smooth step
	//MedianFilterType::Pointer filter=MedianFilterType::New();
	ImageType::SizeType filterRadius;
	filterRadius[0]=filterRadius[1]=smoothFilterValue;
	filter->SetInput(tempImageData);
	filter->SetRadius(filterRadius);
	filter->Update();
//	copyImageData(filter->GetOutput(),tempImageData2);
	tempImageData2=filter->GetOutput();
	smoothFilterFlag=true;
	fusionFlag=false;
//	imageShow(filter->GetOutput());
	imageShow(tempImageData2);
//	writeImage(tempImageData2,"tesss.tif");
}
void ImageProcess::on_pushButton_fusion_clicked()
{
	if(!smoothFilterFlag)
	{
		QMessageBox::information(NULL,"Warning","The smooth filter was not done.");
		fusionFlag=false;
		return;
	}
	pseudocolorProcess(tempImageData2);
	fusionFlag=true;
}

UnChImageType::Pointer ImageProcess::rescaleImage(ImageType::Pointer imageData,PixelType minValue,PixelType maxValue)//scale the value to 0-255
{
	RescaleFilterType::Pointer rescaleFilter=RescaleFilterType::New();
	rescaleFilter->SetInput(imageData);
	rescaleFilter->SetOutputMinimum(minValue);
	rescaleFilter->SetOutputMaximum(maxValue);
	rescaleFilter->Update();
	CastFilterType::Pointer castFilter=CastFilterType::New();
	castFilter->SetInput(rescaleFilter->GetOutput());
	castFilter->Update();
	return castFilter->GetOutput();
}

void ImageProcess::pseudocolorProcess(ImageType::Pointer imageData)
{
	PixelType maxValue;
	PixelType minValue;
	if(fusionFlag)
	{
		colorbarHighValue=ui.spinBoxHighValue->value();
		colorbarLowValue=ui.spinBoxLowValue->value();
	}
	else
	{
		ui.spinBoxHighValue->setValue(colorWindow);//此处以最后读取的灰度图片为准，最大像素点的数值为colorwindow
		ui.spinBoxLowValue->setValue(0);
		colorbarHighValue=ui.spinBoxHighValue->value();
		colorbarLowValue=ui.spinBoxLowValue->value();
	}
	tempImageData3=NULL;
	tempImageData3=ImageType::New();
	copyImageData(tempImageData2,tempImageData3);
//	writeImage(tempImageData3,"tesss.tif");
// 	IteratorType It(tempImageData3,tempImageData3->GetRequestedRegion());
// 	PixelType temp=0;
// 	while(!It.IsAtEnd())
// 	{
// 		temp=It.Get();
// 		if(temp>colorbarHighValue)
// 			temp=colorbarHighValue;
// 		if(temp<colorbarLowValue)
// 			temp=colorbarLowValue;
// 		It.Set(temp);
// 		++It;
// 	}
	
//	unCharImageData=rescaleImage(tempImageData3,minValue,maxValue);
	sliceInputLuminescneceImage(tempImageData3,colorbarHighValue,colorbarLowValue);//将荧光图片做成0-255范围的灰度图，保存为luminescnece8BitImage.tif
//	write8BitImage(unCharImageData,"temp8BitImage.tif");
	//
	unCharImageData=NULL;
	unCharImageData=UnChImageType::New();
	UnChReaderType::Pointer UnChReader=UnChReaderType::New();
	UnChReader->SetFileName("luminescnece8BitImage.tif");
	UnChReader->SetImageIO(tiffIO);
	UnChReader->Update();
	unCharImageData=UnChReader->GetOutput();

	//
	RGBFilterType::Pointer rgbfilter = RGBFilterType::New();
	rgbfilter->SetInput(unCharImageData);
	rgbfilter->SetColormap( RGBFilterType::Jet );
	rgbfilter->Update();
	rgbImageData=NULL;
	rgbImageData=RGBImageType::New();
	rgbImageData=rgbfilter->GetOutput();//the pseudocolor image data
	;//wait
	RGBImageType::Pointer fusionImageData=RGBImageType::New();
	if(photographFlag)
	{
		fusionImage(rgbImageData,photographImage,unCharImageData,fusionImageData);
		rgbImageShow(fusionImageData);
	}
	else
	{
		fusionImageData=rgbImageData;
		rgbImageShow(fusionImageData);
	}
}

void ImageProcess::fusionImage(RGBImageType::Pointer inputRGBImage,UnChImageType::Pointer inputPhotography, UnChImageType::Pointer inputLuminescence,RGBImageType::Pointer outputImage)
{
	outputImage->SetRegions(inputRGBImage->GetRequestedRegion());
	outputImage->CopyInformation(inputRGBImage);
	outputImage->Allocate();

	RGBIteratortype inIt(inputRGBImage,inputRGBImage->GetRequestedRegion());
	UnChIteratorType ItPhoto(inputPhotography,inputPhotography->GetRequestedRegion());
	UnChIteratorType ItLumi(inputLuminescence,inputLuminescence->GetRequestedRegion());
	RGBIteratortype outIt(outputImage,outputImage->GetRequestedRegion());

	RGBPixelType RGBtempValue;
	PixelType tempValue;
	while(!ItLumi.IsAtEnd())
	{
		if(ItLumi.Get()==0)
		{
			RGBtempValue.Set(ItPhoto.Get(),ItPhoto.Get(),ItPhoto.Get());
			outIt.Set(RGBtempValue);
		}
		else
		{
			RGBtempValue=inIt.Get();
			outIt.Set(RGBtempValue);
		}
		++ItPhoto;
		++ItLumi;
		++inIt;
		++outIt;
	}
}


void ImageProcess::on_pushButton_clear_clicked()
{
	;
}
void ImageProcess::on_pushButton_save_clicked()
{
	;
}
void ImageProcess::on_pushButton_quit_clicked()
{
	;
}
ImageType::Pointer ImageProcess::readImage(const char* filename)
{
	ReaderType::Pointer reader=ReaderType::New();
	reader->SetFileName(filename);
	/*ImageIO imio=ImageIO::New();*/
	reader->SetImageIO(tiffIO);
	reader->Update();
	return reader->GetOutput();
}

void ImageProcess::writeImage(ImageType* outputImage, const char* filename)
{
	WriterType::Pointer writer=WriterType::New();
	writer->SetFileName(filename);
	writer->SetImageIO(tiffIO);
	writer->SetInput(outputImage);
	writer->Update();
}
void ImageProcess::writeRGBImage(RGBImageType* outputImage,const char* filename)
{
	RGBWriterType::Pointer writer=RGBWriterType::New();
	writer->SetFileName(filename);
	writer->SetImageIO(tiffIO);
	writer->SetInput(outputImage);
	writer->Update();
}
void ImageProcess::write8BitImage(UnChImageType* outputImage, const char* filename)
{
	UnChWriterType::Pointer writer=UnChWriterType::New();
	writer->SetFileName(filename);
	writer->SetImageIO(tiffIO);
	writer->SetInput(outputImage);
	writer->Update();

}
void ImageProcess::clearImageShow()
{
// 	viewer2->Delete();
// 	connector->Delete();
// 	renderer->Delete();
// 	viewer2=vtkImageViewer2::New();
// 	connector=ConnectorType::New();
// 	renderer=vtkRenderer::New();
// 	ui.qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
// 	renderWindowInteractor=ui.qvtkWidget->GetRenderWindow()->GetInteractor();
// 	renderWindowInteractor->Disable();
	
}
void ImageProcess::imageShow(ImageType::Pointer showImage)
{
	connector->SetInput(showImage);
	connector->Update();
	imageFlip->SetInput(connector->GetOutput());
	imageFlip->SetFilteredAxis(1);
	imageFlip->Update();

	colorLevel=1;
	colorWindow=2;

	ConstIteratorType constIterator(showImage,showImage->GetRequestedRegion());
	for(constIterator.GoToBegin();!constIterator.IsAtEnd();++constIterator)
	{
		if(colorWindow<constIterator.Get())
			colorWindow=constIterator.Get();
	}
	colorLevel=(colorWindow+1)/2;
//	renderer->RemoveAllViewProps();
	viewer2->SetColorWindow(colorWindow);
	viewer2->SetColorLevel(colorLevel);
	viewer2->SetInput(imageFlip->GetOutput());
	viewer2->UpdateDisplayExtent();
	viewer2->SetRenderWindow(ui.qvtkWidget->GetRenderWindow());
	
	viewer2->SetRenderer(renderer);
	viewer2->SetSliceOrientationToXY();
	
	renderer->ResetCamera();
	renderer->DrawOn();
	viewer2->SetSliceOrientationToXY();
	viewer2->SetPosition(0,0);
	//int imageShowSize[2]={(ui.qvtkWidget->size()).width(),(ui.qvtkWidget->size()).height()};
	//viewer2->SetSize(imageShowSize);
	viewer2->Render();
	//ui.qvtkWidget->GetRenderWindow()->Render();

// 	viewer->SetInput(connector->GetOutput());
// 	viewer->GetRenderer()->ResetCamera();
// 	ui.qvtkWidget->SetRenderWindow(viewer->GetRenderWindow());
// 	viewer->SetupInteractor(ui.qvtkWidget->GetRenderWindow()->GetInteractor());
// 	viewer->GetRenderer()->DrawOn();
// 	viewer->SetColorLevel(32768);
// 	viewer->SetColorWindow(65535);
}

void ImageProcess::rgbImageShow(RGBImageType::Pointer showImage)
{

	writeRGBImage(showImage,"tempPseudocolorFile.tif");
	vtkTIFFReader* VTKreader=vtkTIFFReader::New();
	VTKreader->SetFileName("tempPseudocolorFile.tif");
	VTKreader->Update();
	imageFlip->SetInput(VTKreader->GetOutput());
	imageFlip->SetFilteredAxis(1);
	imageFlip->Update();
	renderer->RemoveAllViewProps();
	viewer2=vtkImageViewer2::New();
	viewer2->SetInput(imageFlip->GetOutput());
	viewer2->UpdateDisplayExtent();
	viewer2->SetRenderWindow(ui.qvtkWidget->GetRenderWindow());
	viewer2->SetRenderer(renderer);
	viewer2->SetPosition(0,0);
	renderer->ResetCamera();
	renderer->DrawOn();
	ui.qvtkWidget->GetRenderWindow()->Render();

// 
// 
// 	vtkTIFFReader* VTKreader=vtkTIFFReader::New();
// 	VTKreader->SetFileName("tempPseudocolorFile.tif");
// 	VTKreader->Update();
// 	imageFlip->SetInput(VTKreader->GetOutput());
// 	imageFlip->SetFilteredAxis(1);
// 	imageFlip->Update();
// 	renderer->RemoveAllViewProps();
// 	viewer2->SetInput(imageFlip->GetOutput());
// 	viewer2->UpdateDisplayExtent();
// 	viewer2->SetRenderWindow(ui.qvtkWidget->GetRenderWindow());
// 	viewer2->SetRenderer(renderer);
// 	renderer->ResetCamera();
// 	renderer->DrawOn();
// 	ui.qvtkWidget->GetRenderWindow()->Render();
}

void ImageProcess::copyImageData(ImageType::Pointer inputData,ImageType::Pointer outputData)
{
// 	ImageType::RegionType inputRegion;
// 	ImageType::RegionType::IndexType inputStart;
// 	ImageType::RegionType::SizeType size;
// 	outputData->SetSpacing(inputData->GetSpacing());
	outputData->SetRegions(inputData->GetRequestedRegion());
	outputData->CopyInformation(inputData);
	outputData->Allocate();

	IteratorType outputIt(outputData,outputData->GetRequestedRegion());
 	ConstIteratorType inputIt(inputData,inputData->GetRequestedRegion());
	while(!inputIt.IsAtEnd())
	{
		outputIt.Set(inputIt.Get());
		++inputIt;
		++outputIt;
	}
}
PixelType ImageProcess::minusPixel(PixelType pixelValue,PixelType subValue)
{
	return pixelValue>subValue ? (pixelValue-subValue):0;
}
void ImageProcess::on_spinBoxValueChanged()
{
	PixelType highSpinValue;
	PixelType lowSpinValue;
	highSpinValue=ui.spinBoxHighValue->value();
	lowSpinValue=ui.spinBoxLowValue->value();
	if(highSpinValue<=lowSpinValue)
	{
		QMessageBox::information(NULL,"Warning","highSpinValue should be larger than lowSpinValue, the difference should be 10 or more.");
		ui.spinBoxHighValue->setValue(colorbarHighValue);
		ui.spinBoxLowValue->setValue(colorbarLowValue);
		
	}
	else
	{
		pseudocolorProcess(tempImageData2);
	}

}
void ImageProcess::sliceInputLuminescneceImage(ImageType::Pointer inputImage,PixelType HValue,PixelType LValue)
{
	UnChImageType::Pointer sliced8BitImageData=UnChImageType::New();
	CastFilterType::Pointer castFilter=CastFilterType::New();
	castFilter->SetInput(inputImage);
	castFilter->Update();
	sliced8BitImageData=castFilter->GetOutput();
//	writeImage(inputImage,"okok.tif");//
	float spaceValue=(float)((float)HValue-LValue)/256;
	ConstIteratorType ConstIt(inputImage,inputImage->GetRequestedRegion());
	UnChIteratorType It(sliced8BitImageData,sliced8BitImageData->GetRequestedRegion());
	UnChPixelType temp;
	int temp2;
	while (!It.IsAtEnd())
	{
		temp2=(int)((int)ConstIt.Get()/spaceValue);
		if(temp2>=255)
			temp2=255;
		else
			temp2=temp2+1;
		if (ConstIt.Get()==0)
		{
			temp2=0;
		}
		temp=(UnChPixelType)temp2;
		It.Set(temp);
		++It;
		++ConstIt;
	}
	write8BitImage(sliced8BitImageData,"luminescnece8BitImage.tif");
}