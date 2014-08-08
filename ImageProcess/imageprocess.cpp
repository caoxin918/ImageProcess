#include "imageprocess.h"
#include "QFileDialog"
#include "QMessageBox"
#include <stdio.h>

using namespace std;



//
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

	
	//
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

	//下面是添加伪彩色，但是利用直接的RGBFilterType的Jet colormap，是直接将图片中的所有灰度图进行255种伪彩色添加，即与像素点数据无关，不适合使用，需要重新编写colormap或者是自定义映射函数
	RGBFilterType::Pointer rgbfilter = RGBFilterType::New();
	rgbfilter->SetInput(unCharImageData);
	rgbfilter->SetColormap( RGBFilterType::Jet );
	rgbfilter->Update();
	rgbImageData=NULL;
	rgbImageData=RGBImageType::New();
	rgbImageData=rgbfilter->GetOutput();//the pseudocolor image data
	;//wait

	//

	UnChIteratorType It(unCharImageData,unCharImageData->GetRequestedRegion());
	RGBIteratortype RGBIt(rgbImageData,rgbImageData->GetRequestedRegion());
	//初始化colormap数据
	float colorMapJetData[255][3]=
	{
		0,0,0.5156,
		0,0,0.5313,
		0,0,0.5469,
		0,0,0.5625,
		0,0,0.5781,
		0,0,0.5938,
		0,0,0.6094,
		0,0,0.6250,
		0,0,0.6406,
		0,0,0.6563,
		0,0,0.6719,
		0,0,0.6875,
		0,0,0.7031,
		0,0,0.7188,
		0,0,0.7344,
		0,0,0.7500,
		0,0,0.7656,
		0,0,0.7813,
		0,0,0.7969,
		0,0,0.8125,
		0,0,0.8281,
		0,0,0.8438,
		0,0,0.8594,
		0,0,0.8750,
		0,0,0.8906,
		0,0,0.9063,
		0,0,0.9219,
		0,0,0.9375,
		0,0,0.9531,
		0,0,0.9688,
		0,0,0.9844,
		0,0,1.0000,
		0,0.0156,1.0000,
		0,0.0313,1.0000,
		0,0.0469,1.0000,
		0,0.0625,1.0000,
		0,0.0781,1.0000,
		0,0.0938,1.0000,
		0,0.1094,1.0000,
		0,0.1250,1.0000,
		0,0.1406,1.0000,
		0,0.1563,1.0000,
		0,0.1719,1.0000,
		0,0.1875,1.0000,
		0,0.2031,1.0000,
		0,0.2188,1.0000,
		0,0.2344,1.0000,
		0,0.2500,1.0000,
		0,0.2656,1.0000,
		0,0.2813,1.0000,
		0,0.2969,1.0000,
		0,0.3125,1.0000,
		0,0.3281,1.0000,
		0,0.3438,1.0000,
		0,0.3594,1.0000,
		0,0.3750,1.0000,
		0,0.3906,1.0000,
		0,0.4063,1.0000,
		0,0.4219,1.0000,
		0,0.4375,1.0000,
		0,0.4531,1.0000,
		0,0.4688,1.0000,
		0,0.4844,1.0000,
		0,0.5000,1.0000,
		0,0.5156,1.0000,
		0,0.5313,1.0000,
		0,0.5469,1.0000,
		0,0.5625,1.0000,
		0,0.5781,1.0000,
		0,0.5938,1.0000,
		0,0.6094,1.0000,
		0,0.6250,1.0000,
		0,0.6406,1.0000,
		0,0.6563,1.0000,
		0,0.6719,1.0000,
		0,0.6875,1.0000,
		0,0.7031,1.0000,
		0,0.7188,1.0000,
		0,0.7344,1.0000,
		0,0.7500,1.0000,
		0,0.7656,1.0000,
		0,0.7813,1.0000,
		0,0.7969,1.0000,
		0,0.8125,1.0000,
		0,0.8281,1.0000,
		0,0.8438,1.0000,
		0,0.8594,1.0000,
		0,0.8750,1.0000,
		0,0.8906,1.0000,
		0,0.9063,1.0000,
		0,0.9219,1.0000,
		0,0.9375,1.0000,
		0,0.9531,1.0000,
		0,0.9688,1.0000,
		0,0.9844,1.0000,
		0,1.0000,1.0000,
		0.0156,1.0000,0.9844,
		0.0313,1.0000,0.9688,
		0.0469,1.0000,0.9531,
		0.0625,1.0000,0.9375,
		0.0781,1.0000,0.9219,
		0.0938,1.0000,0.9063,
		0.1094,1.0000,0.8906,
		0.1250,1.0000,0.8750,
		0.1406,1.0000,0.8594,
		0.1563,1.0000,0.8438,
		0.1719,1.0000,0.8281,
		0.1875,1.0000,0.8125,
		0.2031,1.0000,0.7969,
		0.2188,1.0000,0.7813,
		0.2344,1.0000,0.7656,
		0.2500,1.0000,0.7500,
		0.2656,1.0000,0.7344,
		0.2813,1.0000,0.7188,
		0.2969,1.0000,0.7031,
		0.3125,1.0000,0.6875,
		0.3281,1.0000,0.6719,
		0.3438,1.0000,0.6563,
		0.3594,1.0000,0.6406,
		0.3750,1.0000,0.6250,
		0.3906,1.0000,0.6094,
		0.4063,1.0000,0.5938,
		0.4219,1.0000,0.5781,
		0.4375,1.0000,0.5625,
		0.4531,1.0000,0.5469,
		0.4688,1.0000,0.5313,
		0.4844,1.0000,0.5156,
		0.5000,1.0000,0.5000,
		0.5156,1.0000,0.4844,
		0.5313,1.0000,0.4688,
		0.5469,1.0000,0.4531,
		0.5625,1.0000,0.4375,
		0.5781,1.0000,0.4219,
		0.5938,1.0000,0.4063,
		0.6094,1.0000,0.3906,
		0.6250,1.0000,0.3750,
		0.6406,1.0000,0.3594,
		0.6563,1.0000,0.3438,
		0.6719,1.0000,0.3281,
		0.6875,1.0000,0.3125,
		0.7031,1.0000,0.2969,
		0.7188,1.0000,0.2813,
		0.7344,1.0000,0.2656,
		0.7500,1.0000,0.2500,
		0.7656,1.0000,0.2344,
		0.7813,1.0000,0.2188,
		0.7969,1.0000,0.2031,
		0.8125,1.0000,0.1875,
		0.8281,1.0000,0.1719,
		0.8438,1.0000,0.1563,
		0.8594,1.0000,0.1406,
		0.8750,1.0000,0.1250,
		0.8906,1.0000,0.1094,
		0.9063,1.0000,0.0938,
		0.9219,1.0000,0.0781,
		0.9375,1.0000,0.0625,
		0.9531,1.0000,0.0469,
		0.9688,1.0000,0.0313,
		0.9844,1.0000,0.0156,
		1.0000,1.0000,0,
		1.0000,0.9844,0,
		1.0000,0.9688,0,
		1.0000,0.9531,0,
		1.0000,0.9375,0,
		1.0000,0.9219,0,
		1.0000,0.9063,0,
		1.0000,0.8906,0,
		1.0000,0.8750,0,
		1.0000,0.8594,0,
		1.0000,0.8438,0,
		1.0000,0.8281,0,
		1.0000,0.8125,0,
		1.0000,0.7969,0,
		1.0000,0.7813,0,
		1.0000,0.7656,0,
		1.0000,0.7500,0,
		1.0000,0.7344,0,
		1.0000,0.7188,0,
		1.0000,0.7031,0,
		1.0000,0.6875,0,
		1.0000,0.6719,0,
		1.0000,0.6563,0,
		1.0000,0.6406,0,
		1.0000,0.6250,0,
		1.0000,0.6094,0,
		1.0000,0.5938,0,
		1.0000,0.5781,0,
		1.0000,0.5625,0,
		1.0000,0.5469,0,
		1.0000,0.5313,0,
		1.0000,0.5156,0,
		1.0000,0.5000,0,
		1.0000,0.4844,0,
		1.0000,0.4688,0,
		1.0000,0.4531,0,
		1.0000,0.4375,0,
		1.0000,0.4219,0,
		1.0000,0.4063,0,
		1.0000,0.3906,0,
		1.0000,0.3750,0,
		1.0000,0.3594,0,
		1.0000,0.3438,0,
		1.0000,0.3281,0,
		1.0000,0.3125,0,
		1.0000,0.2969,0,
		1.0000,0.2813,0,
		1.0000,0.2656,0,
		1.0000,0.2500,0,
		1.0000,0.2344,0,
		1.0000,0.2188,0,
		1.0000,0.2031,0,
		1.0000,0.1875,0,
		1.0000,0.1719,0,
		1.0000,0.1563,0,
		1.0000,0.1406,0,
		1.0000,0.1250,0,
		1.0000,0.1094,0,
		1.0000,0.0938,0,
		1.0000,0.0781,0,
		1.0000,0.0625,0,
		1.0000,0.0469,0,
		1.0000,0.0313,0,
		1.0000,0.0156,0,
		1.0000,0,0,
		0.9844,0,0,
		0.9688,0,0,
		0.9531,0,0,
		0.9375,0,0,
		0.9219,0,0,
		0.9063,0,0,
		0.8906,0,0,
		0.8750,0,0,
		0.8594,0,0,
		0.8438,0,0,
		0.8281,0,0,
		0.8125,0,0,
		0.7969,0,0,
		0.7813,0,0,
		0.7656,0,0,
		0.7500,0,0,
		0.7344,0,0,
		0.7188,0,0,
		0.7031,0,0,
		0.6875,0,0,
		0.6719,0,0,
		0.6563,0,0,
		0.6406,0,0,
		0.6250,0,0,
		0.6094,0,0,
		0.5938,0,0,
		0.5781,0,0,
		0.5625,0,0,
		0.5469,0,0,
		0.5313,0,0,
		0.5156,0,0
	};
	RGBPixelType rgbPixelTemp;
	UnChPixelType uhPixelTemp;
	while(!It.IsAtEnd())
	{
		uhPixelTemp=It.Get();
		if(uhPixelTemp>0)
		{
			rgbPixelTemp.SetRed((UnChPixelType)(colorMapJetData[uhPixelTemp-1][0] * 255.0));
			rgbPixelTemp.SetGreen((UnChPixelType)(colorMapJetData[uhPixelTemp-1][1] * 255.0));
			rgbPixelTemp.SetBlue((UnChPixelType)(colorMapJetData[uhPixelTemp-1][2] * 255.0));
		}
		else
		{
			rgbPixelTemp.Set(0,0,0);
		}
		RGBIt.Set(rgbPixelTemp);
		++It;
		++RGBIt;
	}


	//
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
// 	CastFilterType::Pointer castFilter=CastFilterType::New();
// 	castFilter->SetInput(inputImage);
// 	castFilter->Update();
// 	sliced8BitImageData=castFilter->GetOutput();
// 	writeImage(inputImage,"okok.tif");//
// 	float spaceValue=(float)((float)HValue-LValue)/256;
// 	ConstIteratorType ConstIt(inputImage,inputImage->GetRequestedRegion());
// 	UnChIteratorType It(sliced8BitImageData,sliced8BitImageData->GetRequestedRegion());
// 	UnChPixelType temp;
// 	int temp2,temp3;
// 	while (!It.IsAtEnd())
// 	{
// 		temp2=ConstIt.Get();
// 		if(temp2>=LValue)
// 		{
// 			temp2=(int)(temp2-lowValue);
// 		}
// 		else
// 		{
// 			temp2=0;
// 		}
// 		temp3=(int)((float)temp2/spaceValue)+1;
// 		if(temp3>=255)
// 			temp2=255;
// 		if(temp3==1)
// 			temp2=0;
// // 		if (ConstIt.Get()<LValue)//将小于colorbar低值的像素点置零
// // 		{
// // 			temp2=0;
// // 		}
// 		temp=(UnChPixelType)temp2;
// 		It.Set(temp);
// 		++It;
// 		++ConstIt;
// 	}

/******************************************************
下面的一段是将荧光图像映射到0-255区间内，得到0-255的灰度图
这里记录下基本原理，以防遗忘：
这里的inputimage是指上一步，也就是滤波处理后生成的图像数据；
首先，自定义一个临时图像数据，tempImage,最后会在这个的基础上
生成8位的灰度图；
然后的while循环有两个目的，1是找到图像中的最大和最小像素点的数值，2是将图像的所有像素点的数值约束在给定的colorbar数值范围内，这里是指当图像的最大值大于colorbar上限，或图像最小值低于colorbar下限的情况
最后，将每个像素点映射到0-255区间中的一个点，该数值是原始图像中像素点之余给点的colorbar，归一到0-255区间后的数值
注意：为了防止设置的colorbar上下区间太大，从而致使每个颜色区间的数值太高，进而致使低像素点被归类到0这一数值，有必要进行相关的矫正（在最后）

	*****************************************************/
	ImageType::Pointer tempImage=ImageType::New();
	tempImage->CopyInformation(inputImage);
	copyImageData(inputImage,tempImage);
	IteratorType tempIt(tempImage,tempImage->GetRequestedRegion());
	PixelType minImageValue,maxImageValue;
	minImageValue=65535;
	maxImageValue=0;
	while(!tempIt.IsAtEnd())
	{
		if(tempIt.Get()>=HValue)
			tempIt.Set(HValue);
		if(tempIt.Get()<=LValue)
			tempIt.Set(LValue);
		if(tempIt.Get()>maxImageValue)
			maxImageValue=tempIt.Get();
		if(tempIt.Get()<minImageValue)
			minImageValue=tempIt.Get();
		++tempIt;
	}
	int sliceMin,sliceMax;
	sliceMax=(int)((maxImageValue-LValue)/(HValue-LValue)*255);
	sliceMin=(int)((minImageValue-LValue)/(HValue-LValue)*255);
	if(sliceMin==0)
		sliceMin=1;//方便进行接下来的矫正
	if(sliceMax<2)
		sliceMax=2;
	sliced8BitImageData=rescaleImage(tempImage,sliceMin,sliceMax);
	UnChIteratorType sliceIt(sliced8BitImageData,sliced8BitImageData->GetRequestedRegion());
	ConstIteratorType inputIt(inputImage,inputImage->GetRequestedRegion());
	while(!inputIt.IsAtEnd())
	{
		if(inputIt.Get()<LValue)
		{
			sliceIt.Set(0);
		}
		++inputIt;
		++sliceIt;
	}
//	writeImage(inputImage,"okok.tif");
	write8BitImage(sliced8BitImageData,"luminescnece8BitImage.tif");
}