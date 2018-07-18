/***********************************************************************
ImageTool - Tool class to load an image into an augmented
reality sandbox.
Copyright (c) 2013-2015 Oliver Kreylos

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

#include "ImageTool.h"

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/OpenFile.h>

#include "Sandbox.h"

/*********************************
Methods of class ImageToolFactory:
*********************************/

ImageToolFactory::ImageToolFactory(Vrui::ToolManager& toolManager)
	:ToolFactory("ImageTool",toolManager),
	 imageSelectionHelper(Vrui::getWidgetManager(),"",".jpg;.png;.tif",Vrui::openDirectory("."))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Set tool class' factory pointer: */
	ImageTool::factory=this;
	}

ImageToolFactory::~ImageToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ImageTool::factory=0;
	}

const char* ImageToolFactory::getName(void) const
	{
	return "Show Image";
	}

const char* ImageToolFactory::getButtonFunction(int) const
	{
	return "Toggle Image";
	}

Vrui::Tool* ImageToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	return new ImageTool(this,inputAssignment);
	}

void ImageToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	delete tool;
	}

/**********************************
Static elements of class ImageTool:
**********************************/

ImageToolFactory* ImageTool::factory=0;

/**************************
Methods of class ImageTool:
**************************/

void ImageTool::loadImageFile(const char* imageFileName)
	{
	/* Load the selected Image file: */
	load(imageFileName);
	
	OGTransform imageT;
	
	/* Calculate an appropriate Image transformation to fit the Image into the sandbox's domain: */
	const Scalar* imageBox=getImageBox();
	Scalar imageSx=imageBox[2]-imageBox[0];
	Scalar imageSy=imageBox[1]-imageBox[3];
	Scalar boxSx=application->bbox.getSize(0);
	Scalar boxSy=application->bbox.getSize(1);
	
	/* Shift the Image's center to the box's center: */
	Point imageCenter;
	imageCenter[0]=Math::mid(imageBox[0],imageBox[2]);
	imageCenter[1]=Math::mid(imageBox[1],imageBox[3]);
	imageCenter[2]=0.0;
	imageT=OGTransform::translateFromOriginTo(imageCenter);
	
	/* Determine whether the Image should be rotated: */
	Scalar scale=Math::min(imageSx/boxSx,imageSy/boxSy);
	Scalar scaleRot=Math::min(imageSx/boxSy,imageSy/boxSx);
	
	if(scale<scaleRot)
		{
		/* Scale and rotate Image: */
		imageT*=OGTransform::rotate(OGTransform::Rotation::rotateZ(Math::rad(Scalar(90))));
		scale=scaleRot;
		}
	
	/* Scale Image without rotation: */
	imageT*=OGTransform::scale(scale);
	
	/* Set the Image transformation: */
	setTransform(imageT*OGTransform(application->boxTransform));
	}

void ImageTool::loadImageFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Load the selected Image file: */
	loadImageFile(cbData->selectedDirectory->getPath(cbData->selectedFileName).c_str());
	}

ImageToolFactory* ImageTool::initClass(Vrui::ToolManager& toolManager)
	{
	/* Create the tool factory: */
	factory=new ImageToolFactory(toolManager);
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

ImageTool::ImageTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment),
	 imageTransform(OGTransform::identity)
	{
	}

ImageTool::~ImageTool(void)
	{
	}

void ImageTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Query Image file name: */
	imageFileName=configFileSection.retrieveString("./imageFileName",imageFileName);
	}
	
void ImageTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write configuration data to given configuration file section: */
	configFileSection.storeString("./imageFileName",imageFileName);
	}

void ImageTool::initialize(void)
	{
	/* Bring up a file selection dialog if there is no pre-configured Image file: */
	if(imageFileName.empty())
		{
		/* Load a Image file: */
		factory->imageSelectionHelper.loadFile("Load Image File...",this,&ImageTool::loadImageFileCallback);
		}
	else
		{
		/* Load the configured Image file: */
		loadImageFile(imageFileName.c_str());
		}
	}

const Vrui::ToolFactory* ImageTool::getFactory(void) const
	{
	return factory;
	}

void ImageTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Toggle this Image tool as the active one: */
		if(!imageFileName.empty())
			application->toggleImage(this);
		}
	}
