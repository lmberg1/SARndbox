/***********************************************************************
EarthquakeTool - Tool to create a circular perturbation in the water table to 
mimic an earthquake.

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

#include "EarthquakeTool.h"

#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <IO/ValueSource.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/OpenFile.h>
#include <Vrui/DisplayState.h>

#include "WaterTable2.h"
#include "EarthquakeManager.h"
#include "Sandbox.h"
#include "Config.h"

/**************************************
Methods of class EarthquakeToolFactory:
**************************************/

EarthquakeToolFactory::EarthquakeToolFactory(WaterTable2* sWaterTable, EarthquakeManager* sEarthquakeManager, Vrui::ToolManager& toolManager)
	:ToolFactory("EarthquakeTool",toolManager),
	 waterTable(sWaterTable), 
	 earthquakeManager(sEarthquakeManager)
	{
	/* Retrieve bathymetry grid and cell sizes: */
	for(int i=0;i<2;++i)
		{
		gridSize[i]=waterTable->getBathymetrySize(i);
		cellSize[i]=waterTable->getCellSize()[i];
		}
	
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Set tool class' factory pointer: */
	EarthquakeTool::factory=this;
	}

EarthquakeToolFactory::~EarthquakeToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	EarthquakeTool::factory=0;
	}

const char* EarthquakeToolFactory::getName(void) const
	{
	return "Create Earthquake";
	}

const char* EarthquakeToolFactory::getButtonFunction(int) const
	{
	return "Create Earthquake";
	}

Vrui::Tool* EarthquakeToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	return new EarthquakeTool(this,inputAssignment);
	}

void EarthquakeToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	delete tool;
	}

/***************************************
Static elements of class EarthquakeTool:
***************************************/

EarthquakeToolFactory* EarthquakeTool::factory=0;

/* Calculate distance between two points */
static double getDistance(int *p1, int *p2) {
	return sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2));
}

/*******************************
Methods of class EarthquakeTool:
*******************************/

void EarthquakeTool::createCircularPerturbation(int *center, int radius, double perturbation) const
	{
	GLfloat *pPtr = bathymetryBuffer;
	/* For all pixel columns */
	for (int i = 0; i < factory->gridSize[0]; i++) {
		pPtr = bathymetryBuffer + i;
		
		/* Iterate through all values in that column */
		for (int j = 0; j < factory->gridSize[1]; j++) {
			pPtr += factory->gridSize[0];
			
			/* Perturb the pixel if it's inside the circle */
			int p2[2] = {i, j};
			if (getDistance(center, p2) <= radius)
				*pPtr += perturbation;
		}
	}
	}
	
int EarthquakeTool::getPixelPos(Vrui::Point p1, const char comp)
{
	if (comp == 'x')
		{
		/* Average the pixel position calculations for the top and bottom 
		corners since they might not be in line */
		double x1 = (p1[0] - basePlaneCorners[1][0])/(basePlaneCorners[0][0] - basePlaneCorners[1][0])*factory->gridSize[0];
		double x2 = (p1[0] - basePlaneCorners[3][0])/(basePlaneCorners[2][0] - basePlaneCorners[3][0])*factory->gridSize[0];
		int x = round((x1 + x2)/2);
		return x;
		}
	if (comp == 'y')
		{
		/* Average the pixel position calculations for the left and right
		corners since they might not be in line */
		double y1 = (p1[1] - basePlaneCorners[3][1])/(basePlaneCorners[1][1] - basePlaneCorners[3][1])*factory->gridSize[1];
		double y2 = (p1[1] - basePlaneCorners[2][1])/(basePlaneCorners[0][1] - basePlaneCorners[2][1])*factory->gridSize[1];
		int y = round((y1 + y2)/2);
		return y;
		}
	
	return 0;
}

EarthquakeToolFactory* EarthquakeTool::initClass(WaterTable2* sWaterTable, EarthquakeManager *sEarthquakeManager, Vrui::ToolManager& toolManager)
	{	
	/* Create the tool factory: */
	factory=new EarthquakeToolFactory(sWaterTable,sEarthquakeManager, toolManager);
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

EarthquakeTool::EarthquakeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment),
	 bathymetryBuffer(new GLfloat[EarthquakeTool::factory->gridSize[1]*EarthquakeTool::factory->gridSize[0]]),
	 requestPending(false)
	{
	
	/* Get the sandbox layout filename */
	std::string sandboxLayoutFileName=CONFIG_CONFIGDIR;
	sandboxLayoutFileName.push_back('/');
	sandboxLayoutFileName.append(CONFIG_DEFAULTBOXLAYOUTFILENAME);

	/* Read the sandbox layout file: */
	{
	IO::ValueSource layoutSource(Vrui::openFile(sandboxLayoutFileName.c_str()));
	layoutSource.skipWs();
	std::string s=layoutSource.readLine();
	for(int i=0;i<4;++i)
		{
		layoutSource.skipWs();
		s=layoutSource.readLine();
		basePlaneCorners[i]=Misc::ValueCoder<OPoint>::decode(s.c_str(),s.c_str()+s.length());
		}
	}
	}

EarthquakeTool::~EarthquakeTool(void)
	{
	delete[] bathymetryBuffer;
	}

const Vrui::ToolFactory* EarthquakeTool::getFactory(void) const
	{
	return factory;
	}

void EarthquakeTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Request a bathymetry grid from the water table: */
		requestPending=factory->waterTable->requestBathymetry(bathymetryBuffer);
		}
	}

void EarthquakeTool::frame(void)
	{
	if(requestPending&&factory->waterTable->haveBathymetry())
		{
		/* Get position of cursor in object space */
		Vrui::Point cursorPos=Vrui::getInverseNavigationTransformation().transform(getButtonDevicePosition(0));
		
		/* Get position and size of perturbation in pixels */
		int radius = factory->earthquakeManager->getEarthquakeRadius();
		int center[2] = {getPixelPos(cursorPos, 'x'), getPixelPos(cursorPos, 'y')};
		
		/* Get perturbation of earthquake */
		double perturbation = factory->earthquakeManager->getEarthquakePerturbation();
		
		// Make sure pixel center is in bounds of the box
		if (center[0] >= 0 && center[0] < factory->gridSize[0] &&
		    center[1] >= 0 && center[1] < factory->gridSize[1])
			{
			/* Update the bathymetry grid: */
			createCircularPerturbation(center, radius, perturbation);

			/* Set bathymetry grid to updated bathymetry buffer */
			factory->earthquakeManager->setBathymetryGrid(bathymetryBuffer);
			}
		
		requestPending=false;
		}
	}
