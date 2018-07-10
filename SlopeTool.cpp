/***********************************************************************
SlopeTool - Tool class to show slope in an augmented
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

#include "SlopeTool.h"

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/OpenFile.h>

#include "Sandbox.h"

/*********************************
Methods of class SlopeToolFactory:
*********************************/

SlopeToolFactory::SlopeToolFactory(Vrui::ToolManager& toolManager)
	:ToolFactory("SlopeTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Set tool class' factory pointer: */
	SlopeTool::factory=this;
	}

SlopeToolFactory::~SlopeToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SlopeTool::factory=0;
	}

const char* SlopeToolFactory::getName(void) const
	{
	return "Show Slope";
	}

const char* SlopeToolFactory::getButtonFunction(int) const
	{
	return "Toggle Slope";
	}

Vrui::Tool* SlopeToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	return new SlopeTool(this,inputAssignment);
	}

void SlopeToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	delete tool;
	}

/**********************************
Static elements of class SlopeTool:
**********************************/

SlopeToolFactory* SlopeTool::factory=0;

/**************************
Methods of class SlopeTool:
**************************/

SlopeToolFactory* SlopeTool::initClass(Vrui::ToolManager& toolManager)
	{
	/* Create the tool factory: */
	factory=new SlopeToolFactory(toolManager);
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

SlopeTool::SlopeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment)
	{
	}

SlopeTool::~SlopeTool(void)
	{
	}

const Vrui::ToolFactory* SlopeTool::getFactory(void) const
	{
	return factory;
	}

void SlopeTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Toggle this Slope tool as the active one: */
		application->toggleSlope();
		}
	}
