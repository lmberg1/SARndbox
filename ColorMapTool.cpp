/***********************************************************************
ColorMapTool - Tool class to shift color map in an augmented
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

#include "ColorMapTool.h"

#include "Sandbox.h"
#include "DepthImageRenderer.h"

/************************************
Methods of class ColorMapToolFactory:
************************************/

ColorMapToolFactory::ColorMapToolFactory(Vrui::ToolManager& toolManager)
	:ToolFactory("ColorMapTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(2);
	
	/* Set tool class' factory pointer: */
	ColorMapTool::factory=this;
	}

ColorMapToolFactory::~ColorMapToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ColorMapTool::factory=0;
	}

const char* ColorMapToolFactory::getName(void) const
	{
	return "Shift Color Map";
	}

const char* ColorMapToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if (buttonSlotIndex == 0)
		return "Shift Color Map Down";
	else return "Shift Color Map Up";
	}

Vrui::Tool* ColorMapToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	return new ColorMapTool(this,inputAssignment);
	}

void ColorMapToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	delete tool;
	}

/*************************************
Static elements of class ColorMapTool:
*************************************/

ColorMapToolFactory* ColorMapTool::factory=0;

/*****************************
Methods of class ColorMapTool:
*****************************/

ColorMapToolFactory* ColorMapTool::initClass(Vrui::ToolManager& toolManager)
	{
	/* Create the tool factory: */
	factory=new ColorMapToolFactory(toolManager);
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

ColorMapTool::ColorMapTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment)
	{
	}

ColorMapTool::~ColorMapTool(void)
	{
	}

const Vrui::ToolFactory* ColorMapTool::getFactory(void) const
	{
	return factory;
	}

void ColorMapTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		float change=0.2;
		Plane p;
		if (buttonSlotIndex == 0)
			{
			p = application->depthImageRenderer->getBasePlane();
			float offset = p.getOffset();
			p.setOffset(offset+change);
			}
		else
			{
			p = application->depthImageRenderer->getBasePlane();
			float offset = p.getOffset();
			p.setOffset(offset-change);
			}
		bool updateTransform=false;
		application->depthImageRenderer->setBasePlane(p, updateTransform);
		}
	}
