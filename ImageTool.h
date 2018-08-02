/***********************************************************************
ImageTool - Tool class to load an image into an augmented
reality sandbox.
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

#ifndef IMAGETOOL_INCLUDED
#define IMAGETOOL_INCLUDED

#include <string>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GLMotif/FileSelectionHelper.h>
#include <Vrui/Tool.h>
#include <Vrui/Application.h>

#include "Types.h"
#include "Image.h"

/* Forward declarations: */
class Sandbox;
class ImageTool;

class ImageToolFactory:public Vrui::ToolFactory
	{
	friend class ImageTool;
	/* Elements: */
	private:
	GLMotif::FileSelectionHelper imageSelectionHelper; // Helper object to load images from files
	
	/* Constructors and destructors: */
	public:
	ImageToolFactory(Vrui::ToolManager& toolManager);
	virtual ~ImageToolFactory(void);
	
	/* Methods from Vrui::ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Vrui::Tool* createTool(const Vrui::ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Vrui::Tool* tool) const;
	};

class ImageTool:public Image,public Vrui::Tool,public Vrui::Application::Tool<Sandbox>
	{
	friend class ImageToolFactory;
	
	/* Elements: */
	private:
	static ImageToolFactory* factory; // Pointer to the factory object for this class
	std::string imageFileName; // Name of image file to load
	OGTransform imageTransform; // The transformation to apply to the image
	
	/* Private methods: */
	void loadImageFile(const char* imageFileName); // Loads an image from a file
	void loadImageFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData); // Called when the user selects an image to load
	
	/* Constructors and destructors: */
	public:
	static ImageToolFactory* initClass(Vrui::ToolManager& toolManager);
	ImageTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~ImageTool(void);
	
	/* Methods from class Vrui::Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

#endif
