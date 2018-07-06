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

#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

#include "Types.h"

class Image:public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId; // ID of texture object holding image
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	int imageSize[2]; // Width and height of the image in pixels
	Scalar imageBox[4]; // Lower-left and upper-right corner coordinates of the image
	float* rgb; // Array of image color values
	OGTransform transform; // Transformation from camera space to image space
	PTransform imageTransform; // Full transformation matrix from camera space to image pixel space
	GLfloat imageTransformMatrix[16]; // Full transformation matrix from camera space to image pixel space to upload to OpenGL
	
	/* Private methods: */
	void calcMatrix(void); // Calculates the camera space to image pixel space transformation
	
	/* Constructors and destructors: */
	public:
	Image(void); // Creates an uninitialized DEM
	virtual ~Image(void);
	
	/* Methods from class GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	void load(const char* imageFileName); // Loads the image from the given file
	const Scalar* getImageBox(void) const // Returns the image's bounding box as lower-left x, lower-left y, upper-right x, upper-right y
		{
		return imageBox;
		}
	void setTransform(const OGTransform& newTransform); // Sets the DEM transformation
	const PTransform& getImageTransform(void) const // Returns the full transformation from camera space to image pixel space
		{
		return imageTransform;
		}
	void bindTexture(GLContextData& contextData) const; // Binds the image texture object to the currently active texture unit
	void uploadImageTransform(GLint location) const; // Uploads the image transformation into the GLSL 4x4 matrix at the given uniform location
	};

#endif
