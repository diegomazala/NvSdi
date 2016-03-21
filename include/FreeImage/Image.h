/**
 ***********************************************************************
 * \class dm::Image
 * \brief A simple class which wrap some functionalities of FreeImage 
 * library in order to load images easier. 
 ***********************************************************************
 * \author		Diego Mazala - diegomazala@gmail.com
 * \date		March, 2009
 ***********************************************************************/


#ifndef __IMAGE_H__
#define __IMAGE_H__


#include "FreeImage.h"
#include <stdexcept>
#include <string>

namespace dm
{

class Image
{

//===========================================================	
public:											// Methods

	//
	//! Constructor a image from file
	Image(const char* pFileName):
		mWidth(0), 
		mHeight(0),
		mpImage(0),
		mNumBits(32)
	{
		this->Load(pFileName);
	}

	
	//
	//! Destructor. Release the memory.
	virtual ~Image()
	{
		ReleaseData();
	}


	//
	//! Load a iamge from file
	virtual void Load(const char* pFileName)
	{
		// Delete any image if it exists
		ReleaseData();

		//Automatically detects the format(from over 20	formats!)
		FREE_IMAGE_FORMAT lImgFormat = FreeImage_GetFileType(pFileName,0);
		mpImage = FreeImage_Load(lImgFormat, pFileName);
		mpImage = FreeImage_ConvertTo32Bits(mpImage);	// mNumBits(32)

		// Check for loading problem
		if(!mpImage)
		{
			std::string lErrStr("<Image::Load> Cannot load image file : ");
			lErrStr += pFileName;
			throw std::exception(lErrStr.c_str());
		}

		mWidth	= FreeImage_GetWidth(mpImage);
		mHeight	= FreeImage_GetHeight(mpImage);
	}


	//
	//! Save an image in file
	virtual void Save(const char* pFileName)
	{
		std::string fileName(pFileName);
		FREE_IMAGE_FORMAT lImgFormat = FreeImage_GetFIFFromFilename(pFileName);
		if(lImgFormat==FIF_UNKNOWN)
		{
			lImgFormat = FIF_PNG;
			fileName += ".png";
		}
						
		if(!FreeImage_Save(lImgFormat, this->mpImage, fileName.c_str(), 0))
		{
			std::string lErrStr("<Image::Save> Cannot save image file : ");
			lErrStr += fileName;
			throw std::exception(lErrStr.c_str());
		}
	}



	//
	//! Save an image in file
	static void Save(const char* pFileName, unsigned char* pImageData, int w, int h, int channels)
	{
		std::string fileName(pFileName);
		FREE_IMAGE_FORMAT lImgFormat = FreeImage_GetFIFFromFilename(pFileName);
		if(lImgFormat==FIF_UNKNOWN)
		{
			lImgFormat = FIF_PNG;
			fileName += ".png";
		}

		FIBITMAP* lpImage = FreeImage_ConvertFromRawBits(pImageData, w, h, channels*w, channels*8, 0xFF0000, 0x00FF00, 0x0000FF, false);
		if(!FreeImage_Save(lImgFormat, lpImage, fileName.c_str(), 0))
		{
			std::string lErrStr("<Image::Save> Cannot save image file : ");
			lErrStr += fileName;
			throw std::exception(lErrStr.c_str());
		}
		FreeImage_Unload(lpImage);
	}


	//
	//! Convert the format from BGRA to RGBA
	virtual void ConvertBgraToRgba()
	{
		if(!mpImage)
			throw std::exception("<ConvertToGLRgba> Image is invalid.");

		unsigned char* pixeles = (unsigned char*)FreeImage_GetBits(mpImage);
		unsigned char temp;

		//FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
		for(int j= 0; j<mWidth*mHeight; j++)
		{
			temp = pixeles[j*4+2];
			pixeles[j*4+2] = pixeles[j*4+0];
			pixeles[j*4+0] = temp;
		}
	}

	//
	//! Convert the format from RGBA to BRGA
	virtual void ConvertRgbaToBgra()
	{
		if(!mpImage)
			throw std::exception("<ConvertToGLRgba> Image is invalid.");

		ConvertBgraToRgba();
	}
	

	//
	//! Return the image pixel data
	unsigned char* Data() const
	{
		return FreeImage_GetBits(mpImage);
	}


	//
	//! Release the image pixel data
	virtual void ReleaseData()
	{
		if(mpImage)
			FreeImage_Unload(mpImage);	
	}


	//
	//! Return the width
	int	Width()  const 
	{ 
		return mWidth;  
	}


	//
	//! Return the height
	int	Height() const 
	{ 
		return mHeight; 
	}


	//
	//! Return the number of bits per pixel
	int	NumBits()const 
	{ 
		return mNumBits;
	}



//===========================================================
protected:										// Attributes

	int			mWidth;		///< Width of the image
	int			mHeight;	///< Height of the image
	FIBITMAP*	mpImage;	///< Data of the image (FreeImage)
	const int	mNumBits;	///< Number of bits: 32

};



} // end namespace 

#endif //__IMAGE_H__
