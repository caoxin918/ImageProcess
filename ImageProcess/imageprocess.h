#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <QtGui/QMainWindow>
#include "ui_imageprocess.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkTIFFImageIO.h"

#include "itkImageRegionIterator.h"

#include "itkMedianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkScalarToRGBColormapImageFilter.h"

#include "itkImageToVTKImageFilter.h"

#include "vtkImageViewer.h"
#include "vtkRenderer.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkImageFlip.h"

#include "vtkTIFFReader.h"


typedef unsigned short PixelType;
const unsigned int Dimension=2;
typedef itk::Image<PixelType,Dimension> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;//输出16bit图像

typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
typedef itk::ImageRegionIterator<ImageType> IteratorType;
typedef itk::TIFFImageIO TIFFIOType;
typedef itk::MedianImageFilter<ImageType,ImageType> MedianFilterType;
typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;

typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;

typedef unsigned char UnChPixelType;//used for pseudocolor process
typedef itk::Image<UnChPixelType,Dimension> UnChImageType;
typedef itk::CastImageFilter<ImageType,UnChImageType> CastFilterType;
typedef itk::RescaleIntensityImageFilter<ImageType,ImageType> RescaleFilterType;
typedef itk::RGBPixel< unsigned char >        RGBPixelType;
typedef itk::Image< RGBPixelType, Dimension > RGBImageType;

typedef itk::ScalarToRGBColormapImageFilter< UnChImageType, RGBImageType> RGBFilterType;

typedef itk::ImageToVTKImageFilter<RGBImageType> RGBConnectorType;
typedef itk::ImageFileWriter<RGBImageType> RGBWriterType;//输出8bit rgb图像
typedef itk::ImageFileWriter<UnChImageType> UnChWriterType;//输出8bit图像
typedef itk::ImageFileReader<UnChImageType> UnChReaderType;
typedef itk::ImageRegionIterator<RGBImageType> RGBIteratortype;
typedef itk::ImageRegionIterator<UnChImageType> UnChIteratorType;
class ImageProcess : public QMainWindow
{
	Q_OBJECT

public:
	ImageProcess(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ImageProcess();

private:
	Ui::ImageProcessClass ui;
	PixelType substractValue;
	PixelType smoothFilterValue;
	PixelType colorbarHighValue;
	PixelType colorbarLowValue;
	bool photographFlag;
	bool luminescenceFlag;
	bool substractFlag;
	bool smoothFilterFlag;
	bool fusionFlag;
	PixelType colorWindow;
	PixelType colorLevel;
	PixelType highValue;
	PixelType lowValue;


private slots:
	void on_pushButton_photograph_clicked();
	void on_pushButton_luminescence_clicked();
	void on_pushButton_substract_clicked();
	void on_pushButton_smooth_clicked();
	void on_pushButton_fusion_clicked();
	void on_pushButton_save_clicked();
	void on_pushButton_clear_clicked();
	void on_pushButton_quit_clicked();
	void on_spinBoxValueChanged();

private:
	ImageType::Pointer readImage(const char* filename);//read a tiff image
	void writeImage(ImageType* outputImage, const char* filename);//write the images to disk
	void writeRGBImage(RGBImageType* outputImage,const char* filename);
	void write8BitImage(UnChImageType* outputImage, const char* filename);
	void initialAll();
	void resetAll();
	void imageShow(ImageType::Pointer showImage);
	void rgbImageShow(RGBImageType::Pointer showImage);
	void clearImageShow();

	void copyImageData(ImageType::Pointer inputData,ImageType::Pointer outputData);
	PixelType minusPixel(PixelType pixelValue,PixelType subValue);
	void pseudocolorProcess(ImageType::Pointer imageData);
	UnChImageType::Pointer rescaleImage(ImageType::Pointer imageData,PixelType minValue,PixelType maxValue);//rescale the image to the value between assigned values
	void fusionImage(RGBImageType::Pointer inputRGBImage,UnChImageType::Pointer inputPhotography, UnChImageType::Pointer inputLuminescence,RGBImageType::Pointer outputImage);

	//sliceInputLuminescneceImage()   函数功能：将输入的16位荧光图，按照上下colorbar映射到0-255,然后存在磁盘上，命名luminescnece8BitImage.tif
	void sliceInputLuminescneceImage(ImageType::Pointer inputImage,PixelType HValue,PixelType LValue);
	//over

private:
	ImageType::Pointer luminescenceImage;
	UnChImageType::Pointer photographImage;
	ImageType::Pointer tempImageData;//a temp image data to process
	ImageType::Pointer tempImageData2;
	ImageType::Pointer tempImageData3;
	TIFFIOType::Pointer tiffIO;

	UnChImageType::Pointer unCharImageData;//used for pseudocolor process
	RGBImageType::Pointer rgbImageData;//used to store the rgb image data


	ImageType::RegionType inputRegion;
	ImageType::RegionType::IndexType inputStart;
	ImageType::RegionType::SizeType size;

	MedianFilterType::Pointer filter;

	vtkRenderer* renderer;
	ConnectorType::Pointer connector;
	vtkImageViewer* viewer;
	vtkImageViewer2* viewer2;
	vtkRenderWindowInteractor* renderWindowInteractor;
	vtkImageFlip* imageFlip;

};

#endif // IMAGEPROCESS_H
