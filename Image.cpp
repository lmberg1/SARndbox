/***********************************************************************
Image - Class to represent images as float-valued texture objects.
Copyright (c) 2013-2016 Oliver Kreylos

This file is part of the Augmented Reality Sandbox (SARndbox).

The Augmented Reality Sandbox is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Augmented Reality Sandbox is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Augmented Reality Sandbox; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include "Image.h"
#include "CImg.h"

#include <IO/File.h>
#include <IO/OpenFile.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBTextureFloat.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLARBTextureRg.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <Geometry/Matrix.h>

#include <stdio.h>

using namespace cimg_library;

/********************************
Methods of class Image::DataItem:
********************************/

Image::DataItem::DataItem(void)
	:textureObjectId(0)
	{
	/* Check for and initialize all required OpenGL extensions: */
	GLARBTextureFloat::initExtension();
	GLARBTextureRectangle::initExtension();
	GLARBTextureRg::initExtension();
	GLARBShaderObjects::initExtension();
	
	/* Create the texture object: */
	glGenTextures(1,&textureObjectId);
	}

Image::DataItem::~DataItem(void)
	{
	/* Destroy the texture object: */
	glDeleteTextures(1,&textureObjectId);
	}

/**********************
Methods of class Image:
**********************/

void Image::calcMatrix(void)
	{
	/* Convert the image transformation into a projective transformation matrix: */
	imageTransform=PTransform(transform);
	PTransform::Matrix& dtm=imageTransform.getMatrix();
	
	/* Pre-multiply the projective transformation matrix with the image space to image pixel space transformation: */
	PTransform image;
	image.getMatrix()(0,0)=Scalar(imageSize[0]-1)/(imageSize[0]);
	image.getMatrix()(0,3)=Scalar(0.5);
	image.getMatrix()(1,1)=Scalar(imageSize[1]-1)/(-imageSize[1]);
	image.getMatrix()(1,3)=Scalar(0.5)+Scalar(imageSize[1]-1);
	image.getMatrix()(2,2)=Scalar(1);
	image.getMatrix()(2,3)=0.0;
	imageTransform.leftMultiply(image);
	
	/* Convert the full transformation to column-major OpenGL format: */
	GLfloat* dtmPtr=imageTransformMatrix;
	for(int j=0;j<4;++j)
		for(int i=0;i<4;++i,++dtmPtr) 
			*dtmPtr=GLfloat(dtm(i,j));
	}

Image::Image(void)
	:rgb(0),
	 transform(OGTransform::identity)
	{
	imageSize[0]=imageSize[1]=0;
	}

Image::~Image(void)
	{
	delete[] rgb;
	}

void Image::initContext(GLContextData& contextData) const
	{
	/* Create and register a data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the image array into the texture object: */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,dataItem->textureObjectId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGB,imageSize[0],imageSize[1],0,GL_RGB,GL_FLOAT,rgb);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
	}

void Image::load(const char* imageFileName)
	{
	/* Read the image file: */
	CImg<float> image(imageFileName);
	imageSize[0] = image.width();
	imageSize[1] = image.height();
	imageBox[0] = 0.0;
	imageBox[1] = imageSize[1];
	imageBox[2] = imageSize[0];
	imageBox[3] = 0.0;
	rgb=new float[imageSize[1]*imageSize[0]*3];
	
	int ind = 0;
	cimg_forY(image, y)
		{
		cimg_forX(image, x)
			{
			rgb[ind++]=image(x, y, 0)/255.0;
			rgb[ind++]=image(x, y, 1)/255.0;
			rgb[ind++]=image(x, y, 2)/255.0;
			}
		}
	calcMatrix();
	}

void Image::bindTexture(GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Bind the image texture: */
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,dataItem->textureObjectId);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGB,imageSize[0],imageSize[1],0,GL_RGB,GL_FLOAT,rgb);
	}
	
void Image::setTransform(const OGTransform& newTransform)
	{
	transform=newTransform;
	
	/* Update the image transformation: */
	calcMatrix();
	}
	
void Image::uploadImageTransform(GLint location) const
	{
	/* Upload the matrix to OpenGL: */
	glUniformMatrix4fvARB(location,1,GL_FALSE,imageTransformMatrix);
	}
