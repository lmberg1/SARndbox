/***********************************************************************
WaterLevelTool - Tool class to increase/decrease the water level in an
augmented reality sandbox.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include "WaterLevelTool.h"

#include <Vrui/ToolManager.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>

#include "Sandbox.h"

/****************************************
Static elements of class WaterLevelTool:
****************************************/

WaterLevelToolFactory* WaterLevelTool::factory=0;

/********************************
Methods of class WaterLevelTool:
********************************/

WaterLevelToolFactory* WaterLevelTool::initClass(Vrui::ToolManager& toolManager)
	{
	/* Create the tool factory: */
	factory=new WaterLevelToolFactory("WaterLevelTool","Manage Water Level",0,toolManager);
	
	/* Set up the tool class' input layout: */
	factory->setNumButtons(2);
	factory->setButtonFunction(0,"Increase Water Level");
	factory->setButtonFunction(1,"Decrease Water Level");
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

WaterLevelTool::WaterLevelTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment), waterLevelStep(0.1)
	{
	}

WaterLevelTool::~WaterLevelTool(void)
	{
	}
	
void WaterLevelTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	waterLevelStep=configFileSection.retrieveValue<Scalar>("./waterLevelStep",waterLevelStep);
	}

const Vrui::ToolFactory* WaterLevelTool::getFactory(void) const
	{
	return factory;
	}

void WaterLevelTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		float newBaseWaterLevel;
		if(buttonSlotIndex == 0)
			// increase water level
			newBaseWaterLevel = application->baseWaterLevel + waterLevelStep;
		else 
			// decrease water level
			newBaseWaterLevel = application->baseWaterLevel - waterLevelStep;
	
		application->baseWaterLevel = newBaseWaterLevel;
		if(application->baseWaterLevelSlider)
			application->baseWaterLevelSlider->setValue(newBaseWaterLevel);
		}
	}
